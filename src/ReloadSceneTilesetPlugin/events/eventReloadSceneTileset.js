export const id = "EVENT_RELOAD_CURRENT_SCENE_TILESET";
export const name = "Reload current scene tileset";
export const groups = ["EVENT_GROUP_SCENE"];

export const autoLabel = (fetchArg) => {
  return `Reload current scene tileset`;
};

export const fields = [
];

export const compile = (input, helpers) => {
  const { _callNative } = helpers;
  
  _callNative("reload_current_scene_tileset");
  
};
