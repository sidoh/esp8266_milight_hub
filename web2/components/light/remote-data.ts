import { RemoteType } from '@/api';

export interface LightCapabilities {
  brightness: boolean;
  color: boolean;
  colorTemp: boolean;
}

export const RemoteTypeDescriptions: Record<RemoteType, string> = {
    rgbw: "Compatible with FUT014, FUT016, FUT103, FUT005, FUT006, FUT007 bulbs.",
    cct: "Compatible with FUT011, FUT017, FUT019 bulbs.",
    rgb_cct: "Compatible with FUT012, FUT013, FUT014, FUT015, FUT103, FUT104, FUT105, and many RGB/CCT LED Strip Controllers.",
    rgb: "Compatible with most RGB LED Strip Controllers.",
    fut089: "Compatible with most newer RGB + dual white bulbs and controllers.",
    fut091: "Compatible with most newer dual white bulbs and controllers.",
    fut020: "Compatible with some RGB LED strip controllers."
};

export const RemoteTypeCapabilities: Record<RemoteType, LightCapabilities> = {
    rgbw: {
        brightness: true,
        color: true,
        colorTemp: false
    },
    cct: {
        brightness: true,
        color: false,
        colorTemp: true
    },
    rgb_cct: {
        brightness: true,
        color: true,
        colorTemp: true
    },
    rgb: {
        brightness: true,
        color: true,
        colorTemp: false
    },
    fut089: {
        brightness: true,
        color: true,
        colorTemp: true
    },
    fut091: {
        brightness: true,
        color: false,
        colorTemp: true
    },
    fut020: {
        brightness: true,
        color: true,
        colorTemp: false
    }
};
