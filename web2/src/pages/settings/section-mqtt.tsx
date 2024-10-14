import * as React from "react";
import {
    NavChildProps
} from "@/components/ui/sidebar-pill-nav";
import {
    FieldSection,
    FieldSections
} from "./form-components";

export const MQTTSettings: React.FC<NavChildProps<"mqtt">> = () => (
  <FieldSections>
    <FieldSection
      title="Connection"
      fields={["mqtt_server", "mqtt_username", "mqtt_password"]}
      fieldTypes={{
        mqtt_password: "password",
      }}
    />
    <FieldSection
      title="Topics"
      fields={[
        "mqtt_topic_pattern",
        "mqtt_update_topic_pattern",
        "mqtt_update_state_pattern",
        "mqtt_client_status_topic",
        "mqtt_retain",
      ]}
      fieldTypes={{
        mqtt_password: "password",
      }}
    />
    <FieldSection
      title="Home Assistant"
      fields={["home_assistant_discovery_prefix"]}
    />
    <FieldSection
      title="Advanced"
      fields={[
        "mqtt_state_rate_limit",
        "mqtt_debounce_delay",
        "simple_mqtt_client_status",
      ]}
    />
  </FieldSections>
);
