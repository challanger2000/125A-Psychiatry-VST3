-- Real REAPER native VST3 GUI, controller, transport, offline and project test.
local R=reaper
local E=assert(os.getenv("PSY_EVIDENCE"))
local helper=assert(os.getenv("PSY_GUI_HELPER"))
local f=assert(io.open(E.."/daw-results.log","w"))
local function log(s) f:write(s.."\n");f:flush() end
local function check(x,s) if not x then error(s,2) end end
local function exists(p) local h=io.open(p,"rb");if h then h:close();return true end;return false end
local function wait(sec)
  local untiltime=R.time_precise()+sec
  repeat coroutine.yield() until R.time_precise()>=untiltime
end
local n=0
local function action(op)
  n=n+1
  local stem=E.."/gui-"..n
  -- Shell executes in parallel with the DAW's main event loop.
  local c=string.format("(/usr/bin/python3 %q %s >%q 2>&1; echo $? >%q) </dev/null &",helper,op,stem..".log",stem..".rc")
  check(os.execute(c),"unable to start GUI action")
  local limit=R.time_precise()+10
  repeat wait(.05) until exists(stem..".rc") or R.time_precise()>limit
  local h=io.open(stem..".rc")
  check(h,"GUI action timeout: "..op)
  local rc=tonumber(h:read("*a"));h:close()
  check(rc==0,"GUI action failure: "..op.." (see "..stem..".log)")
  wait(.25)
end
local names={"DEPRESSION","PARANOIA","SCHIZOPHRENIA","MANIA","PANIC","DISSOCIATION","OBSESSION","HALLUCINATION","PSYCHOSIS","DELIRIUM","CATATONIA","AMNESIA","THERAPY"}
local coords={{480,105},{602,123},{685,202},{702,310},{671,418},{598,501},{480,527},{365,501},{291,419},{264,310},{284,202},{361,123},{129,530}}
local track,fx,params
local function mapParams()
  params={}
  local ok,name=R.TrackFX_GetFXName(track,fx,"")
  check(ok and name:find("125A PSYCHIATRY",1,true),"wrong plugin: "..tostring(name))
  log("Native FX: "..name.." / REAPER "..R.GetAppVersion())
  for i=0,R.TrackFX_GetNumParams(track,fx)-1 do
    local ok,p=R.TrackFX_GetParamName(track,fx,i,"")
    if ok and params[p:upper()]==nil then params[p:upper()]=i end
  end
  for _,p in ipairs(names) do check(params[p]~=nil,"missing parameter "..p) end
end
local function get(p) return R.TrackFX_GetParamNormalized(track,fx,assert(params[p])) end
local function set(p,v)
  check(R.TrackFX_SetParamNormalized(track,fx,assert(params[p]),v),"host set "..p)
end
local function eq(p,v) check(math.abs(get(p)-v)<.001,p.." expected "..v.." got "..get(p)) end
local function guiOpen()
  R.TrackFX_Show(track,fx,3);wait(.6)
  check(R.TrackFX_GetFloatingWindow(track,fx)~=nil,"native floating window missing")
  action("geometry")
end
local function guiClose()
  R.TrackFX_Show(track,fx,2);wait(.3)
  check(R.TrackFX_GetFloatingWindow(track,fx)==nil,"native floating window still open")
end
local function add()
  fx=R.TrackFX_AddByName(track,"VST3: 125A PSYCHIATRY",false,1)
  if fx<0 then fx=R.TrackFX_AddByName(track,"125A PSYCHIATRY",false,1) end
  check(fx>=0,"VST3 discovery/instantiation failed")
  mapParams()
end
local function render(label)
  for k,v in pairs({RENDER_SETTINGS=0,RENDER_BOUNDSFLAG=0,RENDER_STARTPOS=0,RENDER_ENDPOS=4,
    RENDER_CHANNELS=2,RENDER_SRATE=48000,RENDER_TAILFLAG=0,RENDER_ADDTOPROJ=0,
    RENDER_DITHER=0,RENDER_NORMALIZE=0}) do R.GetSetProjectInfo(0,k,v,true) end
  R.GetSetProjectInfo_String(0,"RENDER_FILE",E,true)
  R.GetSetProjectInfo_String(0,"RENDER_PATTERN",label,true)
  R.GetSetProjectInfo_String(0,"RENDER_FORMAT","evaw",true)
  R.GetSetProjectInfo_String(0,"RENDER_FORMAT2","",true)
  R.Main_OnCommand(42230,0)
  local deadline=R.time_precise()+15
  repeat wait(.2) until exists(E.."/"..label..".wav") or R.time_precise()>deadline
  check(exists(E.."/"..label..".wav"),"render missing: "..label)
  wait(1)
end
local function test()
  log("START real DAW")
  check(R.CountTracks(0)==0,"expected isolated empty project")
  R.InsertTrackAtIndex(0,false);track=R.GetTrack(0,0)
  local source=R.PCM_Source_CreateFromFile(E.."/input.wav")
  check(source~=nil,"fixture missing")
  local item=R.AddMediaItemToTrack(track)
  local take=R.AddTakeToMediaItem(item)
  R.SetMediaItemTake_Source(take,source)
  R.SetMediaItemInfo_Value(item,"D_POSITION",0)
  R.SetMediaItemInfo_Value(item,"D_LENGTH",4)
  R.SetMediaItemInfo_Value(item,"D_FADEINLEN",0)
  R.SetMediaItemInfo_Value(item,"D_FADEOUTLEN",0)
  R.GetSetProjectInfo(0,"PROJECT_SRATE",48000,true)
  R.GetSetProjectInfo(0,"PROJECT_SRATE_USE",1,true)
  R.UpdateArrange()
  add()
  guiOpen()
  for i,p in ipairs(names) do
    eq(p,0)
    action("drag "..coords[i][1].." "..coords[i][2])
    check(get(p)>.005,"physical knob drag did not update host "..p)
    action("double "..coords[i][1].." "..coords[i][2])
    eq(p,0)
    log("physical drag/double-click "..p.." PASS")
  end
  -- Product contract: LOSS intentionally activates only 3..6 macros
  -- and clears the others (see testRandomizerRecall).
  for i=1,12 do set(names[i],.85) end
  set("THERAPY",.37);wait(.3)
  action("click 883 576")
  local active,zero=0,0
  for i=1,12 do
    local v=get(names[i])
    check(v>=0 and v<=1,"LOSS out of range "..names[i])
    if v>0 then active=active+1 else zero=zero+1 end
    log("LOSS "..names[i].." = "..v)
  end
  check(active>=3 and active<=6 and zero==12-active,"LOSS active count "..active.." zero "..zero)
  eq("THERAPY",.37)
  log("LOSS OF CONTROL "..active.." active/"..zero.." zero; THERAPY unchanged PASS")
  action("shot motion-a.png");wait(1.6);action("shot motion-b.png");action("motion_on")
  action("click 378 616");wait(2);action("shot still-a.png");wait(1.4);action("shot still-b.png");action("motion_off")
  action("click 378 616");wait(1)
  log("MOTION default on and stopped image PASS")
  local function bypassed()
    return (params.BYPASS~=nil and get("BYPASS")>.5) or not R.TrackFX_GetEnabled(track,fx)
  end
  check(not bypassed(),"initial bypass")
  action("click 263 616");check(bypassed(),"BYPASS click on")
  action("click 263 616");check(not bypassed(),"BYPASS click off")
  log("physical BYPASS toggle PASS")
  action("click 895 175")
  for _,p in ipairs(names) do eq(p,0) end
  log("first-aid global reset 13/13 PASS")
  guiClose()
  for i=1,24 do
    guiOpen();guiClose();log("same REAPER process reopen "..i.."/24 PASS")
  end
  for i=1,4 do
    check(R.TrackFX_Delete(track,fx),"delete prior instance")
    wait(.4);add();guiOpen();guiClose()
    log("same REAPER process destroy/recreate "..i.."/4 PASS")
  end
  for i,p in ipairs(names) do set(p,i*.04) end
  wait(.4)
  local project=E.."/recall.rpp"
  R.Main_SaveProjectEx(0,project,0)
  check(exists(project),"project save failed")
  for _,p in ipairs(names) do set(p,0) end
  R.Main_openProject("noprompt:"..project);wait(1)
  track=R.GetTrack(0,0);fx=0
  check(track and R.TrackFX_GetCount(track)==1,"project reload missing plugin")
  mapParams()
  for i,p in ipairs(names) do eq(p,i*.04) end
  log("real DAW save/reload 13/13 values PASS")
  guiOpen();guiClose()
  R.SetEditCurPos(0,false,false)
  R.OnPlayButton()
  local peak=0;local stop=R.time_precise()+1.5
  repeat wait(.1);peak=math.max(peak,R.Track_GetPeakInfo(R.GetMasterTrack(0),0)) until R.time_precise()>stop
  local pos=R.GetPlayPosition()
  R.OnStopButton();wait(.3)
  check(pos>.5 and peak>.005,"dummy audio transport/peak failed")
  log("real DAW playback PASS, position "..pos.." master peak "..peak)
  for _,p in ipairs(names) do set(p,0) end
  set("THERAPY",1);wait(.3);render("dry")
  set("THERAPY",0);set("PANIC",.72);set("DEPRESSION",.3)
  wait(.3);render("wet")
  action("audio")
  log("real DAW dry/wet PCM comparison PASS")
  R.Main_SaveProjectEx(0,E.."/final-test.rpp",0)
end
local co=coroutine.create(test)
local function tick()
  local ok,err=coroutine.resume(co)
  if not ok then
    log("FAIL "..tostring(err));f:close()
    local h=assert(io.open(E.."/result","w"));h:write("FAIL\n");h:close()
    return
  end
  if coroutine.status(co)=="dead" then
    log("PASS all requested automated REAPER tests");f:close()
    local h=assert(io.open(E.."/result","w"));h:write("PASS\n");h:close()
    R.Main_OnCommand(40004,0)
    return
  end
  R.defer(tick)
end
R.defer(tick)
