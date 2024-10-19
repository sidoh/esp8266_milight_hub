"use strict";

import { schemas } from "@/api/api-zod";
import { z } from "zod";

export interface LightIndexState {
  lights: z.infer<typeof schemas.GatewayListItem>[];
  isLoading: boolean;
}

type Device = Omit<
  z.infer<typeof schemas.GatewayListItem>["device"],
  "id" | "alias"
>;

type Action =
  | {
      type: "UPDATE_STATE";
      device: Device;
      payload: Partial<z.infer<typeof schemas.NormalizedGroupState>>;
    }
  | {
      type: "SET_LIGHTS";
      lights: z.infer<typeof schemas.GatewayListItem>[];
    }
  | {
      type: "DELETE_LIGHT";
      device: Device;
    }
  | {
      type: "ADD_LIGHT";
      device: z.infer<typeof schemas.Alias>;
    }
  | {
      type: "UPDATE_LIGHT_NAME";
      device: z.infer<typeof schemas.GatewayListItem>["device"];
      name: string;
    };

function devicesAreEqual(
  a: Device,
  b: Device
) {
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
