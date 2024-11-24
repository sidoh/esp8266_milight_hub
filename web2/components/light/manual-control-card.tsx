import React, { useState, useEffect } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Input } from "@/components/ui/input";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import { ToggleGroup, ToggleGroupItem } from "@/components/ui/toggle-group";
import { schemas } from "@/api/api-zod";
import {
  RemoteTypeCapabilities,
  RemoteTypeDescriptions,
} from "@/components/light/remote-data";
import { LightControl } from "./light-control";
import { useLightState } from "@/lib/light-state";
import { z } from "zod";
import { parseDeviceId, getGroupCountForRemoteType } from "@/lib/utils";
import { useUpdateState } from "@/hooks/use-update-state";
import { Switch } from "@/components/ui/switch";

function ManualLightCard({
  bulbId,
  state,
  _updateState,
}: {
  bulbId: z.infer<typeof schemas.BulbId>;
  state: z.infer<typeof schemas.NormalizedGroupState>;
  _updateState: (
    newState: Partial<z.infer<typeof schemas.NormalizedGroupState>>
  ) => void;
}) {
  const { updateState } = useUpdateState(bulbId, _updateState);

  return (
    <LightControl
      state={state}
      capabilities={RemoteTypeCapabilities[bulbId.device_type]}
      updateState={updateState}
    />
  );
}

export function ManualControlCard() {
  const { lightStates, dispatch } = useLightState();
  const [deviceId, setDeviceId] = useState("");
  const [deviceType, setDeviceType] = useState("");
  const [groupId, setGroupId] = useState(0);
  const [initialized, setInitialized] = useState(false);

  useEffect(() => {
    if (deviceId && deviceType && groupId) {
      setInitialized(true);
    } else {
      setInitialized(false);
    }
  }, [deviceId, deviceType, groupId]);

  const updateState = (
    newState: Partial<z.infer<typeof schemas.NormalizedGroupState>>
  ) => {
    if (initialized) {
      console.log("updateState", newState);
      dispatch({
        type: "UPDATE_STATE",
        device: {
          device_id: parseDeviceId(deviceId),
          device_type: deviceType,
          group_id: groupId,
        },
        payload: newState,
      });
    }
  };

  const currentLightState = lightStates.lights.find(
    (light) =>
      light.device.device_id === parseDeviceId(deviceId) &&
      light.device.device_type === deviceType &&
      light.device.group_id === groupId
  )?.state || {
    state: "OFF",
    level: 0,
    color_mode: schemas.ColorMode.Values.onoff,
  };

  const handleAliasSelect = (
    light: z.infer<typeof schemas.GatewayListItem>
  ) => {
    setDeviceId(light.device.device_id.toString());
    setDeviceType(light.device.device_type);
    setGroupId(light.device.group_id);
  };

  return (
    <Card className="w-96 min-h-96 flex flex-col">
      <CardHeader className="flex flex-col space-y-2">
        <div className="flex justify-between items-center">
          <CardTitle className="text-lg font-medium">Manual Control</CardTitle>
          {initialized && (
            <Switch
              checked={currentLightState.state === "ON"}
              onCheckedChange={(checked) =>
                updateState({ state: checked ? "ON" : "OFF" })
              }
            />
          )}
        </div>
        <Select
          onValueChange={(value) => {
            const light = lightStates.lights.find(
              (l) =>
                `${l.device.device_id}-${l.device.device_type}-${l.device.group_id}` ===
                value
            );
            if (light) handleAliasSelect(light);
          }}
        >
          <SelectTrigger>
            <SelectValue placeholder="Select a device alias" />
          </SelectTrigger>
          <SelectContent>
            {lightStates.lights.map((light) => (
              <SelectItem
                key={`${light.device.device_id}-${light.device.device_type}-${light.device.group_id}`}
                value={`${light.device.device_id}-${light.device.device_type}-${light.device.group_id}`}
              >
                {light.device.alias ||
                  `${light.device.device_type} Group ${light.device.group_id}`}
              </SelectItem>
            ))}
          </SelectContent>
        </Select>
        <Input
          placeholder="Device ID"
          value={deviceId}
          onChange={(e) => setDeviceId(e.target.value)}
        />
        <Select onValueChange={setDeviceType} value={deviceType}>
          <SelectTrigger>
            <SelectValue placeholder="Select a remote type" />
          </SelectTrigger>
          <SelectContent className="max-w-96">
            {Object.values(schemas.RemoteType.Values).map((type) => (
              <SelectItem key={type} value={type} className="group">
                <div className="flex flex-col items-start max-w-72">
                  <div className="font-medium">{type}</div>
                  <div className="text-sm text-muted-foreground break-words w-full text-left">
                    {RemoteTypeDescriptions[type]}
                  </div>
                </div>
              </SelectItem>
            ))}
          </SelectContent>
        </Select>
        <ToggleGroup
          type="single"
          variant="outline"
          value={groupId.toString()}
          onValueChange={(value) => setGroupId(parseInt(value, 10))}
        >
          {deviceType &&
            Array.from(
              {
                length: getGroupCountForRemoteType(
                  deviceType as z.infer<typeof schemas.RemoteType>
                ),
              },
              (_, i) => (
                <ToggleGroupItem key={i} value={(i + 1).toString()}>
                  {i + 1}
                </ToggleGroupItem>
              )
            )}
        </ToggleGroup>
      </CardHeader>
      {initialized && (
        <CardContent className="flex flex-col flex-grow p-4">
          <ManualLightCard
            bulbId={
              {
                device_id: parseDeviceId(deviceId),
                device_type: deviceType,
                group_id: groupId,
              } as z.infer<typeof schemas.BulbId>
            }
            state={currentLightState}
            _updateState={updateState}
          />
        </CardContent>
      )}
    </Card>
  );
}
