import React, { useEffect, useRef, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Slider } from "@/components/ui/slider";
import { Switch } from "@/components/ui/switch";
import Wheel from "@uiw/react-color-wheel";
import { hsvaToRgba, rgbaToHsva } from "@uiw/color-convert";
import { Sun, Moon, Palette, X, Pencil } from "lucide-react";
import { Button } from "@/components/ui/button";
import { ToggleGroup, ToggleGroupItem } from "@/components/ui/toggle-group";
import { api } from "@/lib/api";
import {
  BulbId,
  ColorMode,
  GatewaysDeviceIdRemoteTypeGroupIdPutRequest,
  GroupStateCommand,
  GroupStateCommandsCommand,
  NormalizedGroupState,
} from "@/api";
import { useRateLimitMerge } from "@/hooks/use-rate-limit-merge";
import { LightStatusIcon } from "./light-status-icon";
import { LightCapabilities, RemoteTypeCapabilities } from "./remote-data";
import { Input } from "@/components/ui/input";

export type NormalizedLightMode = "white" | "color" | "scene" | "night";

export interface LightCardProps {
  name: string;
  state: NormalizedGroupState;
  id: BulbId;
  updateState: (payload: Partial<NormalizedGroupState>) => void;
  onClose?: () => void;
  onNameChange: (newName: string) => void;
}

export function LightCard({
  name,
  state,
  id,
  updateState: _updateState,
  onClose,
  onNameChange,
}: LightCardProps) {
  const [rateLimitedState, setRateLimitedState, clearRateLimitedState] = useRateLimitMerge<GatewaysDeviceIdRemoteTypeGroupIdPutRequest>({}, 500);
  const lastUpdateTimeRef = useRef<number>(0);

  const sendUpdate = async (state: GatewaysDeviceIdRemoteTypeGroupIdPutRequest) => {
    const response = await api.deviceControl.gatewaysDeviceIdRemoteTypeGroupIdPut(
      id.device_id,
      id.device_type,
      id.group_id,
      true,
      "normalized",
      state
    );
    if (response && response.data) {
      _updateState(response.data as Partial<NormalizedGroupState>);
    }
  };

  const updateState = (newState: Partial<NormalizedGroupState>) => {
    _updateState(newState);
    const now = Date.now();
    if (now - lastUpdateTimeRef.current >= 500) {
      // If it's been more than 500ms since the last update, send immediately
      sendUpdate(newState);
      lastUpdateTimeRef.current = now;
      clearRateLimitedState();
    } else {
      // Otherwise, buffer the update
      setRateLimitedState(prevState => ({ ...prevState, ...newState }));
    }
  };

  const sendCommand = async (command: GroupStateCommandsCommand) => {
    return await sendUpdate({ command: command });
  };

  // useEffect to send updates when rateLimitedState changes
  useEffect(() => {
    if (Object.keys(rateLimitedState.value).length > 0) {
      const now = Date.now();
      if (now - lastUpdateTimeRef.current >= 500) {
        sendUpdate(rateLimitedState.value);
        lastUpdateTimeRef.current = now;
        clearRateLimitedState();
      }
    }
  }, [rateLimitedState]);

  const handleSwitchChange = (checked: boolean) => {
    updateState({ state: checked ? "ON" : "OFF" });
  };

  const handleBrightnessChange = (value: number[]) => {
    updateState({ level: value[0] });
  };

  const handleColorTempChange = (value: number[]) => {
    updateState({ kelvin: value[0] });
    _updateState({ color_mode: ColorMode.ColorTemp });
  };

  const handleColorChange = (color: {
    hsva: { h: number; s: number; v: number; a: number };
  }) => {
    const rgba = hsvaToRgba(color.hsva);
    updateState({ color: rgba });
    _updateState({ color_mode: ColorMode.Rgb });
  };

  const convertedColor = rgbaToHsva(
    state.color ? { ...state.color, a: 1 } : { r: 255, g: 255, b: 255, a: 1 }
  );

  const handleModeChange = (value: ColorMode) => {
    _updateState({ color_mode: value });
    if (value === ColorMode.ColorTemp) {
      sendCommand(GroupStateCommand.SetWhite);
    } else if (value === ColorMode.Rgb) {
      updateState({
        color: {
          r: state.color?.r || 255,
          g: state.color?.g || 0,
          b: state.color?.b || 255,
        },
      });
    } else if (value === ColorMode.Onoff) {
      sendCommand(GroupStateCommand.NightMode);
    }
  };

  const capabilities = RemoteTypeCapabilities[id.device_type];

  const [isEditing, setIsEditing] = useState(false);
  const [editedName, setEditedName] = useState(name);

  const handleNameEdit = () => {
    setIsEditing(true);
  };

  const handleNameSave = () => {
    setIsEditing(false);
    onNameChange(editedName);
  };

  return (
    <Card className="w-96 min-h-96 flex flex-col">
      <CardHeader className="flex flex-row items-center justify-between space-y-0 pb-4">
        <div className="flex items-center space-x-2">
          {onClose && (
            <button
              onClick={onClose}
              className="p2 hover:bg-muted border-none hover:border-none"
              aria-label="Close"
            >
              <X size={20} />
            </button>
          )}
          {isEditing ? (
            <Input
              value={editedName}
              onChange={(e) => setEditedName(e.target.value)}
              onBlur={handleNameSave}
              onKeyPress={(e) => e.key === 'Enter' && handleNameSave()}
              className="text-lg font-medium w-40"
            />
          ) : (
            <CardTitle className="text-lg font-medium">{name}</CardTitle>
          )}
          <button
            onClick={isEditing ? handleNameSave : handleNameEdit}
            className="p-1 hover:bg-muted rounded-full"
            aria-label={isEditing ? "Save name" : "Edit name"}
          >
            <Pencil size={16} />
          </button>
          <div
            className="w-6 h-6 rounded-full bg-muted flex items-center justify-center"
            title={`Mode: ${state.color_mode}`}
          >
            <LightStatusIcon state={state} />
          </div>
        </div>
        <div className="flex items-center space-x-2">
          <Switch
            checked={state.state === "ON"}
            onCheckedChange={handleSwitchChange}
            aria-label="Toggle light"
          />
        </div>
      </CardHeader>
      <CardContent className="flex flex-col flex-grow">
        {state.state === "ON" ? (
          <div className="flex flex-col items-center justify-center space-y-4 h-full">
            {capabilities.color && (
              <div className="w-full">
                <div className="flex items-center">
                  <label className="text-sm font-medium ml-2">Color</label>
                </div>
                <div className="mt-2 flex justify-center">
                  <Wheel
                    width={150}
                    height={150}
                    color={convertedColor}
                    onChange={handleColorChange}
                  />
                </div>
              </div>
            )}
            {capabilities.brightness && (
              <div className="w-full">
                <label className="text-sm font-medium">Brightness</label>
                <Slider
                  value={[state.level || 0]}
                  max={100}
                  step={1}
                  className="mt-2"
                  onValueChange={handleBrightnessChange}
                />
              </div>
            )}
            {capabilities.colorTemp && (
              <div className="w-full">
                <label className="text-sm font-medium">Color Temperature</label>
                <Slider
                  value={[state.kelvin || 0]}
                  max={100}
                  step={1}
                  className="mt-2 py-2"
                  onValueChange={handleColorTempChange}
                  gradient="linear-gradient(to right, lightblue, white, orange)"
                />
              </div>
            )}
            <div className="flex flex-col mt-4 w-full">
              <div className="text-sm font-medium">Mode</div>
              <ToggleGroup
                type="single"
                value={state.color_mode}
                onValueChange={handleModeChange}
                aria-label="Select light mode"
                className="justify-normal"
              >
                {capabilities.colorTemp && (
                  <ToggleGroupItem value={ColorMode.ColorTemp}>
                    <Sun size={16} className="mr-2" />
                    White
                  </ToggleGroupItem>
                )}
                {capabilities.color && (
                  <ToggleGroupItem value={ColorMode.Rgb}>
                    <Palette size={16} className="mr-2" />
                    Color
                  </ToggleGroupItem>
                )}
                <ToggleGroupItem value={ColorMode.Onoff}>
                  <Moon size={16} className="mr-2" />
                  Night
                </ToggleGroupItem>
              </ToggleGroup>
            </div>
          </div>
        ) : (
          <div className="flex flex-col items-center justify-center flex-grow">
            <p className="text-muted-foreground">Light is off</p>
          </div>
        )}
        <div className="flex-grow"></div>
        <div className="flex justify-end space-x-4 mt-4">
          <Button size="sm" onClick={() => sendCommand(GroupStateCommand.Pair)}>
            Pair
          </Button>
          <Button
            variant="destructive"
            size="sm"
            onClick={() => sendCommand(GroupStateCommand.Unpair)}
          >
            Unpair
          </Button>
        </div>
      </CardContent>
    </Card>
  );
}
