# P028a — geometry review draft

P027g source `fb416405fedc29e47de7a9ee78dc2b786ecd8afe` passed all four workflows: [DSP](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45482658707), [EditorHost](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45482658731), [Windows](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45482658749), [recorded](https://github.com/challanger2000/125A-Psychiatry-VST3/actions/runs/45482658775). Functional baseline frozen at v0.9.24. No final DAW or musical approval.

Owner requests step-by-step visual review. Create editable VSTGUI geometry draft and matching SVG preview before faceplate generation. Draft is deliberately not compiled or installed; existing plugin and DSP untouched. The XML's circular elements are rectangular placeholders. Logical size 1280×960; Revision B after owner feedback: 12 identical 100 px macro controls on an ellipse with radii 455×335 around (640,465), window outer Ø550, inner Ø490. Therapy and randomizer moved into lower corners; top readouts above the knobs. This is smaller than aspirational 60–65% width to keep actual clock positions clear. THERAPY Ø120; fan upper right; logo upper left. Zoom steps 50/75/100/125/150/200 proposed, not implemented. No artwork placement or final textures until owner approves geometry. Previous generated faceplate not accepted for geometry. Transfer binary artwork by correctly structured ZIP if direct upload unavailable. No new CI needed to assess this non-runtime draft.

Files: psychiatry-layout-draft.uidesc, geometry.svg, geometry.json.

## Revision C

Owner confirmed outer diameter 550 px. Keep 1280×960, 100 px macros, 120 px THERAPY. Free-form symmetric clock positions closer to observation frame, uniform label above and percentage below. Bold title 44 px; zoom placeholder reserved in bottom right next to language. Therapy and LOSS remain separated; no new artwork. Geometry bounds, control rectangles and circular window clearances checked, not a runtime/editor screenshot. Await owner review.

## Revision D — header refinement

Owner liked C substantially better and confirmed header fine-tuning. Title reserve 700×54, 52 px font; 18 px gap to DEPRESSION label (previous 6). Observation window remains Ø550 and shifts 28 px down; macros shift 24 px (OBSESSION 28 px). Side groups, therapy, controls, zoom and brand unchanged. Bounds, rectangular control collisions and circle clearances pass; no compiled GUI or final art yet. Review D before rendering.
