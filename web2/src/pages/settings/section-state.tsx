import * as React from "react";
import { useState, useEffect } from "react";
import { NavChildProps } from "@/components/ui/sidebar-pill-nav";
import {
    FieldSection,
    FieldSections
} from "./form-components";
import Settings from ".";
import { z } from "zod";
import { schemas } from "@/api";
import { useFormContext } from "react-hook-form";
import {
    FormControl,
    FormDescription,
    FormField,
    FormItem,
    FormLabel,
} from "@/components/ui/form";
import SimpleSelect from "@/components/ui/select-box";

type Settings = z.infer<typeof schemas.Settings>;
type GroupStateField = z.infer<typeof schemas.GroupStateField>;

const stateFieldPresets: Record<string, GroupStateField[]> = {
  HomeAssistant: [
    "state",
    "brightness",
    "computed_color",
    "mode",
    "color_temp",
    "color_mode",
  ],
  Custom: [],
};

const GroupStateFieldsPreview: React.FC = () => {
  const form = useFormContext<Settings>();
  const [preview, setPreview] = useState<string>("");

  useEffect(() => {
    const selectedFields = form.watch("group_state_fields");
    const previewObject: Record<string, any> = {};

    selectedFields?.forEach((field: GroupStateField) => {
      switch (field) {
        case "state":
        case "status":
          previewObject[field] = "ON";
          break;
        case "brightness":
          previewObject[field] = 75;
          break;
        case "level":
          previewObject[field] = 191;
          break;
        case "hue":
          previewObject[field] = 180;
          break;
        case "saturation":
          previewObject[field] = 100;
          break;
        case "color":
          previewObject[field] = { r: 0, g: 255, b: 255 };
          break;
        case "mode":
          previewObject[field] = 1;
          break;
        case "kelvin":
          previewObject[field] = 100;
          break;
        case "color_temp":
          previewObject[field] = 370;
          break;
        case "bulb_mode":
          previewObject[field] = "white";
          break;
        case "computed_color":
          previewObject["color"] = { r: 255, g: 255, b: 255 };
          break;
        case "effect":
          previewObject[field] = "1";
          break;
        case "device_id":
          previewObject[field] = 1;
          break;
        case "group_id":
          previewObject[field] = 1;
          break;
        case "device_type":
          previewObject[field] = "rgb_cct";
          break;
        case "oh_color":
          previewObject["color"] = "0,255,255";
          break;
        case "hex_color":
          previewObject["color"] = "#00FFFF";
          break;
        case "color_mode":
          previewObject[field] = "rgb";
          break;
      }
    });

    setPreview(JSON.stringify(previewObject, null, 2));
  }, [form.watch("group_state_fields")]);

  return (
    <div className="flex flex-col gap-2 mt-4">
      <div className="text-sm font-medium">Preview</div>
      <pre className="text-sm text-muted-foreground">{preview}</pre>
    </div>
  );
};

const GroupStateFieldsSelector: React.FC<{}> = ({}) => {
  const form = useFormContext<Settings>();
  const [selectedPreset, setSelectedPreset] = useState<string>("Custom");

  useEffect(() => {
    const currentFields = new Set(form.getValues("group_state_fields"));
    for (const [preset, fields] of Object.entries(stateFieldPresets)) {
      if (isSetEqual(currentFields, new Set(fields))) {
        setSelectedPreset(preset);
        break;
      }
    }
  }, []);

  const isSetEqual = (a: Set<GroupStateField>, b: Set<GroupStateField>) =>
    a.size === b.size && [...a].every((value) => b.has(value));

  const handlePresetChange = (value: string) => {
    setSelectedPreset(value);
    if (value !== "Custom") {
      form.setValue("group_state_fields", stateFieldPresets[value], {
        shouldDirty: true,
        shouldValidate: true,
        shouldTouch: true,
      });
    }
  };

  return (
    <div className="mt-4 flex flex-col gap-4">
      <FormField
        control={form.control}
        name="group_state_fields_preset"
        render={() => (
          <FormItem>
            <FormLabel>Preset</FormLabel>
            <FormControl>
              <SimpleSelect
                options={Object.keys(stateFieldPresets).map((preset) => ({
                  label: preset,
                  value: preset,
                }))}
                value={{ label: selectedPreset, value: selectedPreset }}
                onChange={(option) =>
                  handlePresetChange(option?.value as string)
                }
              />
            </FormControl>
            <FormDescription>
              Customize the fields sent in MQTT state updates and in REST API
              responses. If you're using HomeAssistant, use the preset to ensure
              compatibility.
            </FormDescription>
          </FormItem>
        )}
      />

      {selectedPreset === "Custom" && (
        <FormField
          key={"group_state_fields"}
          control={form.control}
          name={"group_state_fields"}
          render={({ field }) => (
            <FormItem>
              <FormLabel>Custom Fields</FormLabel>
              <FormControl>
                <SimpleSelect
                  isMulti
                  options={Object.entries(schemas.GroupStateField.Values).map(
                    ([key, value]) => ({
                      label: key,
                      value: key,
                    })
                  )}
                  value={field.value?.map((value) => ({
                    label: value,
                    value,
                  }))}
                  onChange={(value) => {
                    field.onChange(value.map((v) => v.value));
                  }}
                />
              </FormControl>
            </FormItem>
          )}
        />
      )}
      <GroupStateFieldsPreview />
    </div>
  );
};

export const StateSettings: React.FC<NavChildProps<"state">> = () => (
  <FieldSections>
    <FieldSection title="🔧 State Fields" fields={[]}>
      <GroupStateFieldsSelector />
    </FieldSection>
    <FieldSection
      title="🔍 Miscellaneous"
      fields={[
        "enable_automatic_mode_switching",
        "default_transition_period",
        "state_flush_interval",
      ]}
    />
  </FieldSections>
);
