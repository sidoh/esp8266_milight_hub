import React, { useEffect, useRef, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Slider } from "@/components/ui/slider";
import { Switch } from "@/components/ui/switch";
import Wheel from "@uiw/react-color-wheel";
import { hsvaToRgba, rgbaToHsva } from "@uiw/color-convert";
import { Sun, Moon, Palette, X, Pencil } from "lucide-react";
import { Button } from "@/components/ui/button";
import { ToggleGroup, ToggleGroupItem } from "@/components/ui/toggle-group";
import { useUpdateState } from "@/hooks/use-update-state";
import { LightStatusIcon } from "./light-status-icon";
import { RemoteTypeCapabilities } from "./remote-data";
import { Input } from "@/components/ui/input";
import { z } from "zod";
import { schemas } from "@/api/api-zod";
import { LightControl } from "./light-control";

export type NormalizedLightMode = "white" | "color" | "scene" | "night";

export interface LightCardProps {
  name: string;
  state: z.infer<typeof schemas.NormalizedGroupState>;
  id: z.infer<typeof schemas.BulbId>;
  updateState: (
    payload: Partial<z.infer<typeof schemas.NormalizedGroupState>>
  ) => void;
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
  const { updateState } = useUpdateState(id, _updateState);
  const [isEditing, setIsEditing] = useState(false);
  const [editedName, setEditedName] = useState(name);
  const handleSwitchChange = (checked: boolean) => {
    updateState({ state: checked ? "ON" : "OFF" });
  };

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
              onKeyPress={(e) => e.key === "Enter" && handleNameSave()}
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
        <LightControl
          state={state}
          capabilities={RemoteTypeCapabilities[id.device_type]}
          updateState={updateState}
        />
      </CardContent>
    </Card>
  );
}
