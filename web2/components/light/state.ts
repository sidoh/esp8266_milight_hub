"use strict";

import { schemas } from "@/api/api-zod";
import { z } from "zod";

export interface LightIndexState {
  lights: (z.infer<typeof schemas.GatewayListItem> & { ephemeral: boolean })[];
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
      type: "UPDATE_ALL_STATE";
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

function devicesAreEqual(a: Device, b: Device) {
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
      const lightExists = state.lights.some((light) =>
        devicesAreEqual(light.device, action.device)
      );

      if (lightExists) {
        return {
          ...state,
          lights: state.lights.map((light) =>
            devicesAreEqual(light.device, action.device)
              ? { ...light, state: { ...light.state, ...action.payload } }
              : light
          ),
        };
      } else {
        const newLight: z.infer<typeof schemas.GatewayListItem> & {
          ephemeral: boolean;
        } = {
          device: { ...action.device } as z.infer<typeof schemas.GatewayListItem>["device"],
          state: { ...action.payload },
          ephemeral: true,
        };
        return {
          ...state,
          lights: [...state.lights, newLight],
        };
      }
    case "UPDATE_ALL_STATE":
      return {
        ...state,
        lights: state.lights.map((light) => ({
          ...light,
          state: action.payload,
        })),
      };
    case "SET_LIGHTS":
      return {
        ...state,
        lights: action.lights.map((light) => ({
          ...light,
          ephemeral: false,
        })),
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
      const device = {
        id: action.device.id!,
        device_id: action.device.device_id!,
        device_type: action.device.device_type!,
        group_id: action.device.group_id!,
        alias: action.device.alias!,
      };
      return {
        ...state,
        lights: [...state.lights, { device, state: { state: "OFF" }, ephemeral: false }],
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
