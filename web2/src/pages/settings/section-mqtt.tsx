import * as React from "react";
import { NavChildProps } from "@/components/ui/sidebar-pill-nav";
import { FieldSection, FieldSections } from "./form-components";
import { FieldValues, useFormContext } from "react-hook-form";
import { useState, useEffect } from "react";
import {
  FormField,
  FormItem,
  FormLabel,
  FormControl,
  FormDescription,
} from "@/components/ui/form";
import SimpleSelect from "@/components/ui/select-box";
import { schemas } from "@/api";
import { z } from "zod";

type Settings = z.infer<typeof schemas.Settings>;
type SettingsKey = keyof typeof schemas.Settings.shape;

const TOPIC_PRESETS: Record<string, Partial<Settings>> = {
  Default: {
    mqtt_topic_pattern: "milight/commands/:device_id/:device_type/:group_id",
    mqtt_update_topic_pattern: "",
    mqtt_state_topic_pattern: "milight/state/:device_id/:device_type/:group_id",
    mqtt_client_status_topic: "milight/client_status",
    simple_mqtt_client_status: true,
  },
  Custom: {},
};

const TopicFieldsSelector: React.FC<{}> = ({}) => {
  const form = useFormContext();
  const [selectedPreset, setSelectedPreset] = useState<string>("Custom");

  useEffect(() => {
    const currentFields = form.getValues();
    for (const [preset, fields] of Object.entries(TOPIC_PRESETS)) {
      if (isPresetEqual(currentFields, fields)) {
        setSelectedPreset(preset);
        break;
      }
    }
  }, []);

  const isPresetEqual = (a: FieldValues, b: Partial<Settings>) =>
    Object.keys(b).every((key) => a[key] === b[key]);

  const handlePresetChange = (value: string) => {
    setSelectedPreset(value);
    if (value !== "Custom") {
      const topicFields = TOPIC_PRESETS[value];
      for (const [key, value] of Object.entries(topicFields)) {
        form.setValue(key as SettingsKey, value as string, {
          shouldDirty: true,
          shouldValidate: true,
          shouldTouch: true,
        });
      }
      form.handleSubmit((data) => {
        console.log(data);
      })();
    }
  };

  return (
    <div className="mt-4 flex flex-col gap-4">
      <FormField
        control={form.control}
        name="topic_fields_preset"
        render={() => (
          <FormItem>
            <FormLabel>Preset</FormLabel>
            <FormControl>
              <SimpleSelect
                options={Object.keys(TOPIC_PRESETS).map((preset) => ({
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
              Customize the MQTT topic patterns. Use the "Default" preset for
              standard configurations.
            </FormDescription>
          </FormItem>
        )}
      />

      {selectedPreset !== "Custom" && (
        <div className="preview-fields">
          <h4 className="text-sm font-medium">Preview:</h4>
          <ul>
            {Object.entries(TOPIC_PRESETS[selectedPreset]).map(
              ([key, value]) => (
                <li key={key} className="mt-2">
                  <div>
                    <strong className="text-sm font-medium">
                      {key
                        .replace(/_/g, " ")
                        .replace(/\b\w/g, (char) => char.toUpperCase())}
                      :
                    </strong>
                  </div>
                  <div>
                    <code className="bg-muted text-sm rounded">
                      {(value as any).toString()}
                    </code>
                  </div>
                  <div className="text-sm text-muted-foreground">
                    {schemas.Settings.shape[key as SettingsKey].description}
                  </div>
                </li>
              )
            )}
          </ul>
        </div>
      )}

      {selectedPreset === "Custom" && (
        <FieldSection
          fields={[
            "mqtt_topic_pattern",
            "mqtt_update_topic_pattern",
            "mqtt_state_topic_pattern",
            "mqtt_client_status_topic",
            "simple_mqtt_client_status",
          ]}
        />
      )}
    </div>
  );
};

export const MQTTSettings: React.FC<NavChildProps<"mqtt">> = () => (
  <FieldSections>
    <FieldSection
      title="MQTT Connection"
      fields={["mqtt_server", "mqtt_username", "mqtt_password"]}
      fieldTypes={{
        mqtt_password: "password",
      }}
    />
    <FieldSection title="MQTT Topics" fields={[]}>
      <TopicFieldsSelector />
    </FieldSection>
    <FieldSection
      title="Home Assistant MQTT Discovery"
      fields={["home_assistant_discovery_prefix"]}
    />
    <FieldSection
      title="Advanced"
      fields={["mqtt_state_rate_limit", "mqtt_debounce_delay", "mqtt_retain"]}
    />
  </FieldSections>
);
