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
        "rf24_channels",
        "rf24_listen_channel",
      ]}
    />
    <FieldSection
      title="🔁 Repeats"
      fields={["packet_repeats", "packet_repeats_per_loop", "listen_repeats"]}
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
