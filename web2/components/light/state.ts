"use strict";

import {
  Alias,
  GatewayListItem,
  GatewayListItemDevice,
  NormalizedGroupState,
} from "@/api";

export interface LightIndexState {
  lights: GatewayListItem[];
  isLoading: boolean;
}

type Action =
  | {
      type: "UPDATE_STATE";
      device: GatewayListItemDevice;
      payload: Partial<NormalizedGroupState>;
    }
  | {
      type: "SET_LIGHTS";
      lights: GatewayListItem[];
    }
  | {
      type: "DELETE_LIGHT";
      device: GatewayListItemDevice;
    }
  | {
      type: "ADD_LIGHT";
      device: Alias;
    }
  | {
      type: "UPDATE_LIGHT_NAME";
      device: GatewayListItemDevice;
      name: string;
    };

function devicesAreEqual(a: GatewayListItemDevice, b: GatewayListItemDevice) {
  return (
    a.device_id === b.device_id &&
    a.device_type === b.device_type &&
    a.group_id === b.group_id
  );
}

// Reducer function
export function reducer(
  state: LightIndexState,
  action: Action
): LightIndexState {
  switch (action.type) {
    case "UPDATE_STATE":
      return {
        ...state,
        lights: state.lights.map((light) =>
          devicesAreEqual(light.device, action.device)
            ? { ...light, state: { ...light.state, ...action.payload } }
            : light
        ),
      };
    case "SET_LIGHTS":
      return {
        ...state,
        lights: action.lights,
        isLoading: false,
      };
    case "DELETE_LIGHT":
      return {
        ...state,
        lights: state.lights.filter(
          (light) => !devicesAreEqual(light.device, action.device)
        ),
      };
    case "ADD_LIGHT":
      console.log(action.device);
      const device = {
        id: action.device.id!,
        device_id: action.device.device_id!,
        device_type: action.device.device_type!,
        group_id: action.device.group_id!,
        alias: action.device.alias!,
      };
      return {
        ...state,
        lights: [...state.lights, { device, state: { state: "OFF" } }],
      };
    case "UPDATE_LIGHT_NAME":
      return {
        ...state,
        lights: state.lights.map((light) =>
          devicesAreEqual(light.device, action.device)
            ? { ...light, device: { ...light.device, alias: action.name } }
            : light
        ),
      };
    default:
      return state;
  }
}
