import * as React from "react";
import { NavChildProps } from "@/components/ui/sidebar-pill-nav";
import { FieldSection, FieldSections, Settings } from "./form-components";
import { useForm, useFormContext } from "react-hook-form";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { ToggleGroup, ToggleGroupItem } from "@/components/ui/toggle-group";
import { Trash2, Plus } from "lucide-react";

const PROTOCOL_OPTIONS = [
  { value: "5", label: "v5" },
  { value: "6", label: "v6" },
];

type GatewayConfig = number[];

export const GatewayConfigsSettings: React.FC = () => {
  const { setValue, getValues } = useFormContext<Settings>();
  const [configs, setConfigs] = React.useState<GatewayConfig[]>(
    () => getValues("gateway_configs") || []
  );
  const [isDirty, setIsDirty] = React.useState(false);

  const addConfig = () => {
    setConfigs([...configs, [0, 0, 6]]);
    setIsDirty(true);
  };

  const removeConfig = (index: number) => {
    setConfigs(configs.filter((_, i) => i !== index));
    setIsDirty(true);
  };

  const updateConfig = (index: number, field: 0 | 1 | 2, value: number) => {
    const newConfigs = [...configs];
    newConfigs[index][field] = value;
    setConfigs(newConfigs);
    setIsDirty(true);
  };

  const saveConfigs = () => {
    setValue("gateway_configs", configs, {
      shouldValidate: true,
      shouldDirty: true,
      shouldTouch: true,
    });
    setIsDirty(false);
  };

  return (
    <FieldSections>
      <FieldSection title="Gateway Configurations" fields={[]}>
        <div className="grid grid-cols-[3fr_3fr_3fr_1fr] gap-2 mb-2 font-semibold">
          <div>Remote ID</div>
          <div>UDP Port</div>
          <div>Protocol</div>
          <div>
            <Button
              onClick={addConfig}
              variant="secondary"
              size="icon"
              className="rounded-full"
              aria-label="Add gateway config"
            >
              <Plus className="h-4 w-4" />
            </Button>
          </div>
        </div>

        {/* Gateway config fields */}
        {configs.map((config, index) => (
          <div
            key={index}
            className="grid grid-cols-[3fr_3fr_3fr_1fr] gap-2 mb-2 items-center"
          >
            <Input
              type="number"
              value={config[0]}
              onChange={(e) => updateConfig(index, 0, parseInt(e.target.value))}
              placeholder="Remote ID"
            />
            <Input
              type="number"
              value={config[1]}
              onChange={(e) => updateConfig(index, 1, parseInt(e.target.value))}
              placeholder="UDP Port"
            />
            <ToggleGroup
              type="single"
              value={config[2].toString()}
              onValueChange={(value) => updateConfig(index, 2, parseInt(value))}
            >
              {PROTOCOL_OPTIONS.map((option) => (
                <ToggleGroupItem key={option.value} value={option.value}>
                  {option.label}
                </ToggleGroupItem>
              ))}
            </ToggleGroup>
            <div className="flex justify-center">
              <Button
                onClick={() => removeConfig(index)}
                variant="ghost"
                size="icon"
                className="text-red-500 hover:text-red-700 hover:bg-red-100"
                aria-label="Remove gateway config"
              >
                <Trash2 className="h-4 w-4" />
              </Button>
            </div>
          </div>
        ))}

        {/* Updated Add and Save buttons */}
        <div className="flex justify-between mt-2">
          <Button onClick={saveConfigs} disabled={!isDirty}>
            Save Changes
          </Button>
        </div>

        <div className="text-sm text-muted-foreground mt-4">
          <p>
            Add servers which mimic the UDP protocol used by official Milight
            gateways. You should only use this if you're trying to integrate
            with a device or service that requires it. MQTT and the REST API are
            more reliable!
          </p>
        </div>
      </FieldSection>
    </FieldSections>
  );
};

export const UDPSettings: React.FC<NavChildProps<"udp">> = () => (
  <FieldSections>
    <FieldSection title="Discovery" fields={["discovery_port"]} />
    <GatewayConfigsSettings />
  </FieldSections>
);
