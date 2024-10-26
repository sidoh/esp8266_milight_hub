import * as React from "react";
import { NavChildProps } from "@/components/ui/sidebar-pill-nav";
import { FieldSection, FieldSections } from "./form-components";

export const NetworkSettings: React.FC<NavChildProps<"network">> = () => (
  <FieldSections>
    <FieldSection
      title="🔒 Security"
      fields={["admin_username", "admin_password"]}
      fieldTypes={{
        admin_password: "password",
      }}
    />
    <FieldSection
      title="📶 WiFi"
      fields={[
        "hostname",
        "wifi_static_ip",
        "wifi_static_ip_gateway",
        "wifi_static_ip_netmask",
        "wifi_mode",
      ]}
      fieldNames={{
        wifi_static_ip: "Static IP",
        wifi_static_ip_gateway: "Static IP Gateway",
        wifi_static_ip_netmask: "Static IP Netmask",
      }}
    />
  </FieldSections>
);
