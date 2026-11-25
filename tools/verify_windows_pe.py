"""Bounded static PE32+ structure and VST3 DLL export/import validation."""
import struct

def inspect_pe(data):
    def need(condition, message):
        if not condition:
            raise ValueError(message)
    def u16(at):
        return struct.unpack_from("<H", data, at)[0]
    def u32(at):
        return struct.unpack_from("<I", data, at)[0]
    need(len(data)>=64 and data[:2]==b"MZ","missing DOS header")
    pe=u32(0x3c)
    need(64<=pe<=len(data)-24 and data[pe:pe+4]==b"PE\0\0","missing PE signature")
    coff=pe+4
    need(u16(coff)==0x8664,"not AMD64")
    sections=u16(coff+2); optional=u16(coff+16)
    need(1<=sections<=96 and optional>=128 and u16(coff+18)&0x2000,"not executable DLL")
    opt=coff+20; table=opt+optional
    need(table+40*sections<=len(data) and u16(opt)==0x20b and u32(opt+108)>=2,"invalid PE32+ header")
    maps=[]
    for i in range(sections):
        s=table+40*i; size=u32(s+16); raw=u32(s+20); rva=u32(s+12)
        need(raw+size<=len(data),"section exceeds file")
        if size: maps.append((rva,size,raw))
    def offset(rva,count=1):
        for base,size,raw in maps:
            if base<=rva and rva-base+count<=size:
                return raw+rva-base
        raise ValueError("unmapped RVA")
    def name(rva):
        at=offset(rva); end=data.find(b"\0",at,min(at+512,len(data)))
        need(end>at,"unterminated PE name")
        return data[at:end].decode("ascii")
    export_rva,export_size=struct.unpack_from("<II",data,opt+112)
    need(export_rva and export_size>=40,"missing export table")
    e=offset(export_rva,40)
    functions,names=u32(e+20),u32(e+24)
    need(0<names<=functions<=4096,"invalid export counts")
    fn=offset(u32(e+28),4*functions); ns=offset(u32(e+32),4*names); ords=offset(u32(e+36),2*names)
    exports=set()
    for i in range(names):
        ordinal=u16(ords+2*i)
        need(ordinal<functions,"invalid export ordinal")
        target=u32(fn+4*ordinal)
        need(target and not export_rva<=target<export_rva+export_size,"invalid or forwarded VST3 export")
        offset(target)
        exports.add(name(u32(ns+4*i)))
    need({"GetPluginFactory","InitDll","ExitDll"}<=exports,"required VST3 exports absent")
    irva,isize=struct.unpack_from("<II",data,opt+120)
    need(irva and isize>=20 and isize<=1024*1024,"missing or invalid import directory")
    imports=[]
    for i in range(min(isize//20,4096)):
        at=offset(irva+20*i,20)
        desc=struct.unpack_from("<IIIII",data,at)
        if not any(desc):break
        need(desc[3]!=0 and desc[4]!=0,"invalid import descriptor")
        dll=name(desc[3]);need(dll.lower().endswith(".dll"),"invalid imported DLL name")
        offset(desc[4],8);imports.append(dll)
    else:raise ValueError("unterminated import table")
    need(imports,"empty import list")
    return exports,imports
