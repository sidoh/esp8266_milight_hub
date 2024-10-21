import React from "react";
import { Lightbulb } from "lucide-react";
import { schemas } from "@/api";
import { z } from "zod";

type NormalizedGroupState = z.infer<typeof schemas.NormalizedGroupState>;

interface LightStatusIconProps {
  state: NormalizedGroupState;
}

export function LightStatusIcon({ state }: LightStatusIconProps) {
  let color = "gray"; // Default color for OFF state
  const colorMode = state.color_mode || "white";

  if (state.state === "ON") {
    if (colorMode === "rgb" && state.color) {
      color = `rgba(${state.color.r}, ${state.color.g}, ${state.color.b}, 1)`;
    } else if (
      state.color_mode === "color_temp" &&
      state.kelvin !== undefined
    ) {
      const kelvin = state.kelvin;
      if (kelvin < 50) {
        color = "lightblue"; // Cool white for low kelvin values
      } else {
        color = "orange"; // Warm white for high kelvin values
      }
    } else if (colorMode === "white") {
      color = "yellow"; // Yellowish in white mode
    }
  }

  return <Lightbulb size={24} style={{ color }} />;
}
