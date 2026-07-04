import * as React from "react";
import { useFormContext } from "react-hook-form";
import { z } from "zod";
import { NavChildProps } from "@/components/ui/sidebar-pill-nav";
import { FieldSection, FieldSections } from "./form-components";
import { Button } from "@/components/ui/button";
import { Spinner } from "@/components/ui/spinner";
import { useToast } from "@/hooks/use-toast";
import { schemas, api } from "@/api";
import { useSettings } from "@/lib/settings";
import {
  Signal,
  SignalHigh,
  SignalLow,
  SignalMedium,
  SignalZero,
} from "lucide-react";

type Settings = z.infer<typeof schemas.Settings>;
type WifiNetwork = { ssid: string; rssi: number };

const rssiIcon = (rssi: number) => {
  if (rssi >= -55) return <SignalHigh className="h-4 w-4" />;
  if (rssi >= -70) return <SignalMedium className="h-4 w-4" />;
  if (rssi >= -80) return <SignalLow className="h-4 w-4" />;
  if (rssi >= -90) return <SignalZero className="h-4 w-4" />;
  return <Signal className="h-4 w-4" />;
};

const WifiScanner: React.FC = () => {
  const { toast } = useToast();
  const [scanning, setScanning] = React.useState(false);
  const [networks, setNetworks] = React.useState<WifiNetwork[] | null>(null);

  const handleScan = async () => {
    setScanning(true);
    setNetworks(null);
    try {
      const res = await fetch("/wifi/scan");
      if (!res.ok) {
        throw new Error(`Server returned ${res.status}`);
      }
      const data = (await res.json()) as WifiNetwork[];
      const cleaned = data
        .filter((n) => n && typeof n.ssid === "string" && n.ssid.length > 0)
        .sort((a, b) => b.rssi - a.rssi);
      setNetworks(cleaned);
    } catch (err) {
      toast({
        title: "WiFi scan failed",
        description:
          err instanceof Error ? err.message : "An unknown error occurred.",
        variant: "destructive",
      });
      setNetworks([]);
    } finally {
      setScanning(false);
    }
  };

  return (
    <div className="space-y-3 mt-2">
      <div className="flex items-center gap-3">
        <Button
          type="button"
          variant="secondary"
          onClick={handleScan}
          disabled={scanning}
        >
          {scanning ? <Spinner className="mr-2" /> : null}
          {scanning ? "Scanning..." : "Scan WiFi Networks"}
        </Button>
      </div>

      {networks !== null && !scanning && (
        <div className="rounded-md border">
          {networks.length === 0 ? (
            <p className="p-3 text-sm text-muted-foreground">
              No networks found.
            </p>
          ) : (
            <ul className="divide-y">
              {networks.map((n, i) => (
                <li key={`${n.ssid}-${i}`}>
                  <span className="flex w-full items-center justify-between px-3 py-2 text-left text-sm">
                    <span className="flex items-center gap-2 truncate">
                      {rssiIcon(n.rssi)}
                      <span className="truncate">{n.ssid}</span>
                    </span>
                    <span className="text-xs text-muted-foreground tabular-nums">
                      {n.rssi} dBm
                    </span>
                  </span>
                </li>
              ))}
            </ul>
          )}
        </div>
      )}
    </div>
  );
};

const EthernetSaveButton: React.FC = () => {
  const { toast } = useToast();
  const form = useFormContext<Settings>();
  const [saving, setSaving] = React.useState(false);

  const handleSave = async () => {
    setSaving(true);
    try {
      const values = form.getValues();
      await api.putSettings({
        wifi_static_ip: values.wifi_static_ip ?? "",
        wifi_static_ip_gateway: values.wifi_static_ip_gateway ?? "",
        wifi_static_ip_netmask: values.wifi_static_ip_netmask ?? "",
      });
      form.reset(form.getValues());
      toast({
        title: "Ethernet settings saved",
        description: "Rebooting so the new IP takes effect...",
        variant: "default",
      });
      await api.postSystem({ command: "restart" });
    } catch (err) {
      toast({
        title: "Failed to save Ethernet settings",
        description:
          err instanceof Error ? err.message : "An unknown error occurred.",
        variant: "destructive",
      });
    } finally {
      setSaving(false);
    }
  };

  return (
    <div className="mt-4">
      <Button type="button" onClick={handleSave} disabled={saving}>
        {saving ? <Spinner className="mr-2" /> : null}
        Save
      </Button>
    </div>
  );
};

export const NetworkSettings: React.FC<NavChildProps<"network">> = () => {
  const { about } = useSettings();
  const isEthernet = about?.ethernet === true;

  return (
    <FieldSections>
      <FieldSection
        title="🔒 Security"
        fields={["admin_username", "admin_password"]}
        fieldTypes={{
          admin_password: "password",
        }}
      />
      {isEthernet ? (
        <FieldSection
          title="🔌 Ethernet"
          description="Configure a static IP for the wired connection, or leave Static IP blank to use DHCP. Gateway is required when using a static IP. The device reboots after saving."
          fields={[
            "hostname",
            "wifi_static_ip",
            "wifi_static_ip_gateway",
            "wifi_static_ip_netmask",
          ]}
          fieldNames={{
            wifi_static_ip: "Static IP (blank for DHCP)",
            wifi_static_ip_gateway: "Gateway",
            wifi_static_ip_netmask: "Netmask",
          }}
        >
          <EthernetSaveButton />
        </FieldSection>
      ) : (
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
        >
          <WifiScanner />
        </FieldSection>
      )}
    </FieldSections>
  );
};
