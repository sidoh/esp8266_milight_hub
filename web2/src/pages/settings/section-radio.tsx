import * as React from "react";
import { NavChildProps } from "@/components/ui/sidebar-pill-nav";
import { FieldSection, FieldSections } from "./form-components";

export const RadioSettings: React.FC<NavChildProps<"radio">> = () => (
  <FieldSections>
    <FieldSection
      title="📻 Device"
      fields={[
        "radio_interface_type",
        "rf24_power_level",
        "rf24_channels"
      ]}
    />
    <FieldSection
      title="📡 Listening"
      fields={["rf24_listen_channel", "ignored_listen_protocols", "listen_repeats"]}
    />
    <FieldSection
      title="🔁 Repeats"
      fields={["packet_repeats", "packet_repeats_per_loop"]}
    />
    <FieldSection
      title="⏱️ Throttling"
      fields={[
        "packet_repeat_throttle_sensitivity",
        "packet_repeat_throttle_threshold",
        "packet_repeat_minimum",
      ]}
    />
  </FieldSections>
);
