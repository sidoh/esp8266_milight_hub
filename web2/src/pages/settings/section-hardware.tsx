import * as React from "react";
import {
    NavChildProps
} from "@/components/ui/sidebar-pill-nav";
import {
    FieldSection,
    FieldSections
} from "./form-components";

export const HardwareSettings: React.FC<NavChildProps<"hardware">> = () => (
  <FieldSections>
    <FieldSection
      title="⚙️ Radio Pins"
      fields={["ce_pin", "csn_pin", "reset_pin"]}
      fieldNames={{
        ce_pin: "Chip Enable (CE) Pin",
        csn_pin: "Chip Select Not (CSN) Pin",
        reset_pin: "Reset Pin",
      }}
    />
    <FieldSection
      title="💡 LED"
      fields={[
        "led_pin",
        "led_mode_operating",
        "led_mode_packet",
        "led_mode_wifi_config",
        "led_mode_wifi_failed",
        "led_mode_packet_count",
      ]}
      fieldNames={{
        led_pin: "LED Pin",
        led_mode_operating: "LED Mode: Idle",
        led_mode_packet: "LED Mode: Packet Sent/Received",
        led_mode_wifi_config: "LED Mode: WiFi in Config Mode",
        led_mode_wifi_failed: "LED Mode: WiFi Connection Failed",
        led_mode_packet_count: "LED Packet Blink Count",
      }}
    />
  </FieldSections>
);
