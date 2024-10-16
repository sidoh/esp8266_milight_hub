import * as React from "react";
import { NavChildProps } from "@/components/ui/sidebar-pill-nav";
import { FieldSection, FieldSections } from "./form-components";
import { Button } from "@/components/ui/button";
import { api } from "@/api";
import { useToast } from "@/hooks/use-toast";
import { Input } from "@/components/ui/input";

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

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    console.log(event.target.files);
    const file = event.target.files?.[0];
    setBackupFile(file || null);
  };

  console.log(backupFile);

  const handleUploadBackup = async () => {
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
        <h3 className="text-lg font-medium">Restore Backup</h3>
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
              type="submit"
              disabled={!backupFile}
              onClick={handleUploadBackup}
            >
              Upload Backup
            </Button>
          </div>
        </form>
      </div>
    </div>
  );
};

export const SystemSettings: React.FC<NavChildProps<"system">> = () => (
  <FieldSections>
    <FieldSection title="Backups" fields={[]}>
      <BackupsSection />
    </FieldSection>
    <FieldSection title="Reboot" fields={["auto_restart_period"]}>
      <ActionSection />
    </FieldSection>
  </FieldSections>
);
