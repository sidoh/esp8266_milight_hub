import * as React from "react";
import { NavChildProps } from "@/components/ui/sidebar-pill-nav";
import {
  DynamicFormControl,
  FieldSection,
  FieldSections,
  StandardFormField,
} from "./form-components";

const GroupStateFieldsSelector: React.FC<{}> = ({}) => {
  return (
    <StandardFormField field="group_state_fields" className="mt-4">
      <DynamicFormControl field="group_state_fields" />
    </StandardFormField>
  );
};

export const SystemSettings: React.FC<NavChildProps<"system">> = () => (
  <FieldSections>
    <FieldSection title="State" fields={["state_flush_interval"]}>
      <GroupStateFieldsSelector />
    </FieldSection>
    <FieldSection title="Restarts" fields={["auto_restart_period"]} />
    <FieldSection
      title="Miscellaneous"
      fields={["enable_automatic_mode_switching", "default_transition_period"]}
    />
  </FieldSections>
);
