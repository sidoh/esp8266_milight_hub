import * as React from "react";
import { NavChildProps } from "@/components/ui/sidebar-pill-nav";
import { FieldSection, FieldSections } from "./form-components";
import { Button } from "@/components/ui/button";
import { api, schemas } from "@/api";
import { useToast } from "@/hooks/use-toast";
import { Input } from "@/components/ui/input";
import { Alert, AlertDescription, AlertTitle } from "@/components/ui/alert";
import { AlertTriangle } from "lucide-react";
import { useEffect, useState } from "react";
import { Skeleton } from "@/components/ui/skeleton";
import { useSettings } from "@/lib/settings";

type SystemInfo = Awaited<ReturnType<typeof api.getAbout>>;

function compareVersions(v1: string, v2: string): number {
  const v1Parts = v1.split(".").map(Number);
  const v2Parts = v2.split(".").map(Number);

  for (let i = 0; i < Math.max(v1Parts.length, v2Parts.length); i++) {
    const v1Part = v1Parts[i] || 0;
    const v2Part = v2Parts[i] || 0;
    if (v1Part > v2Part) return 1;
    if (v1Part < v2Part) return -1;
  }
  return 0;
}

const ActionSection: React.FC = () => {
  const { toast } = useToast();

  const handleReboot = async () => {
    try {
      toast({
        title: "Reboot initiated",
        description: "The device will restart shortly.",
        variant: "default",
      });

      const response = await api.postSystem({
        command: "restart",
      });

      if (!response.success) {
        toast({
          title: "Error initiating reboot",
          description: response.error,
          variant: "destructive",
        });
      }
    } catch (error) {
      if (error instanceof Error) {
        toast({
          title: "Error initiating reboot",
          description: error.message,
          variant: "destructive",
        });
      } else {
        toast({
          title: "Error initiating reboot",
          description: "An unknown error occurred.",
          variant: "destructive",
        });
      }
    }
  };

  return (
    <div className="space-y-2 mt-10">
      <Button variant="destructive" onClick={handleReboot}>
        Reboot Now
      </Button>
    </div>
  );
};

const BackupsSection: React.FC = () => {
  const { toast } = useToast();
  const [backupFile, setBackupFile] = React.useState<File | null>(null);
  const [busy, setBusy] = React.useState(false);

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    console.log(event.target.files);
    const file = event.target.files?.[0];
    setBackupFile(file || null);
  };

  const handleUploadBackup = async () => {
    setBusy(true);
    toast({
      title: "Uploading backup",
      description: "Please wait while your backup is uploaded.",
      variant: "default",
    });

    if (!backupFile) return;

    try {
      const response = await api.postBackup({ file: backupFile });
      if (response.success) {
        toast({
          title: "Success",
          description: response.message,
          variant: "default",
        });
      } else {
        toast({
          title: "Error uploading backup",
          description: response.message,
          variant: "destructive",
        });
      }
    } catch (error) {
      toast({
        title: "Error uploading backup",
        description:
          error instanceof Error ? error.message : "An unknown error occurred",
        variant: "destructive",
      });
    } finally {
      setBackupFile(null);
      setBusy(false);
    }
  };

  return (
    <div className="space-y-4">
      <p className="text-sm text-muted-foreground">
        Backups contain configuration data and devices you've registered with
        the hub. It does not contain states of lights.
      </p>
      <div className="space-y-2">
        <h3 className="text-lg font-medium">Create Backup</h3>
        <Button variant="secondary" asChild>
          <a href="/backup" download="espmh-backup.bin">
            Download Backup
          </a>
        </Button>
      </div>
      <div className="space-y-2">
        <h3 className="text-lg font-medium mt-10">Restore Backup</h3>
        <form onSubmit={handleUploadBackup}>
          <div className="flex items-center space-x-2">
            <Input
              type="file"
              id="backupFile"
              onChange={handleFileChange}
              value={backupFile ? undefined : ""}
              accept=".bin"
              className="flex-grow"
            />
            <Button
              disabled={!backupFile || busy}
              onClick={(e) => {
                e.preventDefault();
                handleUploadBackup();
              }}
              variant="secondary"
            >
              Upload Backup
            </Button>
          </div>
        </form>
      </div>
    </div>
  );
};

const FirmwareSection: React.FC<{
  currentVersion: string | null;
  variant: string | null;
}> = ({ currentVersion, variant }) => {
  const { toast } = useToast();
  const [firmwareFile, setFirmwareFile] = React.useState<File | null>(null);
  const [isChecking, setIsChecking] = React.useState(false);
  const [busy, setBusy] = React.useState(false);
  const [latestVersionInfo, setLatestVersionInfo] = React.useState<{
    version: string;
    url: string;
    body: string;
    download_links: {
      name: string;
      url: string;
    }[];
    release_date: string;
  } | null>(null);

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    setFirmwareFile(file || null);
  };

  const handleUploadFirmware = async () => {
    setBusy(true);
    toast({
      title: "Update started",
      description: "Do not turn off the device until the update is complete.",
      variant: "default",
    });

    api
      .postFirmware({ file: firmwareFile })
      .then(() => {
        toast({
          title: "Success",
          description: "The update is complete. The device will restart.",
          variant: "default",
        });
      })
      .catch((error) => {
        toast({
          title: "Error uploading firmware",
          description: error.message,
          variant: "destructive",
        });
      })
      .finally(() => {
        setBusy(false);
      });
  };

  const checkLatestVersion = async () => {
    setIsChecking(true);
    try {
      const response = await fetch(
        "https://api.github.com/repos/sidoh/esp8266_milight_hub/releases/latest"
      );
      const data = await response.json();
      setLatestVersionInfo({
        version: data.tag_name,
        url: data.html_url,
        body: data.body,
        download_links: data.assets.map((asset: any) => ({
          name: asset.name,
          url: asset.browser_download_url,
        })),
        release_date: data.published_at,
      });
    } catch (error) {
      toast({
        title: "Error checking latest version",
        description: "Failed to fetch the latest version from GitHub.",
        variant: "destructive",
      });
    } finally {
      setIsChecking(false);
    }
  };

  const isNewVersionAvailable = React.useMemo(() => {
    if (!currentVersion || !latestVersionInfo) return false;
    return compareVersions(latestVersionInfo.version, currentVersion) > 0;
  }, [currentVersion, latestVersionInfo]);

  const getDownloadLink = React.useMemo(() => {
    if (!latestVersionInfo || !variant) return null;
    return latestVersionInfo.download_links.find((link) =>
      link.name.toLowerCase().includes(variant.toLowerCase())
    );
  }, [latestVersionInfo, variant]);

  console.log(variant, latestVersionInfo);

  return (
    <div className="space-y-4">
      <Alert variant="destructive">
        <AlertTriangle className="h-4 w-4" />
        <AlertTitle>Warning</AlertTitle>
        <AlertDescription>
          Always create a backup before updating firmware!
        </AlertDescription>
      </Alert>

      <div className="space-y-2">
        <h3 className="text-lg font-medium">Upload Firmware</h3>
        <form onSubmit={handleUploadFirmware}>
          <div className="flex items-center space-x-2">
            <Input
              type="file"
              id="firmwareFile"
              onChange={handleFileChange}
              value={firmwareFile ? undefined : ""}
              accept=".bin"
              className="flex-grow"
            />
            <Button
              disabled={!firmwareFile || busy}
              onClick={(e) => {
                e.preventDefault();
                handleUploadFirmware();
              }}
              variant="secondary"
            >
              Upload Firmware
            </Button>
          </div>
        </form>
      </div>

      {!latestVersionInfo && (
        <div className="space-y-2">
          <h3 className="text-lg font-medium">Check for Updates</h3>
          <div className="flex items-center space-x-2">
            <Button
              onClick={checkLatestVersion}
              disabled={isChecking}
              variant="secondary"
            >
              {isChecking ? "Checking..." : "Check Latest Version"}
            </Button>
          </div>
        </div>
      )}

      {latestVersionInfo && (
        <div className="space-y-2 border p-4 rounded-md">
          <h3 className="text-lg font-medium">Latest Version Information</h3>
          <hr className="my-4" />
          {isNewVersionAvailable && (
            <p className="text-green-600 font-semibold">
              A new version is available!
            </p>
          )}
          <p>
            <strong>Version:</strong> {latestVersionInfo.version}
          </p>
          <p>
            <strong>Release Date:</strong>{" "}
            {new Date(latestVersionInfo.release_date).toLocaleString()}
          </p>
          <p>
            <strong>Release Notes:</strong>
          </p>
          <pre className="whitespace-pre-wrap text-sm bg-muted p-2 rounded-md">
            {latestVersionInfo.body}
          </pre>
          <div className="space-x-2">
            <Button asChild variant="outline">
              <a
                href={latestVersionInfo.url}
                target="_blank"
                rel="noopener noreferrer"
              >
                View on GitHub
              </a>
            </Button>
            {getDownloadLink && (
              <Button asChild variant="secondary">
                <a href={getDownloadLink.url} download>
                  Download Firmware
                </a>
              </Button>
            )}
          </div>
        </div>
      )}
    </div>
  );
};

const SystemInfoSection: React.FC<{
  systemInfo: SystemInfo | null;
  isLoading: boolean;
}> = ({ systemInfo, isLoading }) => {
  if (isLoading) {
    return (
      <div className="space-y-2">
        <Skeleton className="h-4 w-[200px]" />
        <Skeleton className="h-4 w-[150px]" />
        <Skeleton className="h-4 w-[180px]" />
        <Skeleton className="h-4 w-[160px]" />
      </div>
    );
  }

  return systemInfo ? (
    <div className="space-y-2">
      <div className="flex">
        <strong className="w-40">Firmware:</strong> {systemInfo?.firmware}
      </div>
      <div className="flex">
        <strong className="w-40">Version:</strong> {systemInfo?.version}
      </div>
      <div className="flex">
        <strong className="w-40">IP Address:</strong> {systemInfo?.ip_address}
      </div>
      <div className="flex">
        <strong className="w-40">Variant:</strong> {systemInfo?.variant}
      </div>
      <div className="flex">
        <strong className="w-40">Free Heap:</strong> {systemInfo?.free_heap}{" "}
        bytes
      </div>
      <div className="flex">
        <strong className="w-40">Arduino Version:</strong>{" "}
        {systemInfo?.arduino_version}
      </div>
      <div className="flex">
        <strong className="w-40">Last Reset Reason:</strong>{" "}
        {systemInfo?.reset_reason}
      </div>
      <div className="flex">
        <strong className="w-40">Dropped Packets:</strong>{" "}
        {systemInfo?.queue_stats?.dropped_packets}
      </div>
    </div>
  ) : (
    <> </>
  );
};

export const SystemSettings: React.FC<NavChildProps<"system">> = () => {
  const { about, isLoadingAbout } = useSettings();

  return (
    <FieldSections>
      <FieldSection title="🖥️ System Information" fields={[]}>
        <SystemInfoSection systemInfo={about} isLoading={isLoadingAbout} />
      </FieldSection>
      <FieldSection title="🔧 Firmware" fields={[]}>
        <FirmwareSection
          currentVersion={about?.version ?? null}
          variant={about?.variant ?? null}
        />
      </FieldSection>
      <FieldSection title="💾 Backups" fields={[]}>
        <BackupsSection />
      </FieldSection>
      <FieldSection title="🔄 Reboot" fields={["auto_restart_period"]}>
        <ActionSection />
      </FieldSection>
    </FieldSections>
  );
};
