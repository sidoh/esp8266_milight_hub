import { schemas } from "@/api/api-zod";
import { type ClassValue, clsx } from "clsx"
import { twMerge } from "tailwind-merge"
import { z } from "zod";

export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}

export const parseDeviceId = (deviceId: string): number => {
  return deviceId.startsWith("0x")
    ? parseInt(deviceId, 16)
    : parseInt(deviceId, 10);
};

export const getGroupCountForRemoteType = (
  remoteType: z.infer<typeof schemas.RemoteType>
): number => {
  // Stub function - replace with actual logic
  switch (remoteType) {
    case schemas.RemoteType.Values.fut089:
      return 8;
    case schemas.RemoteType.Values.rgb:
      return 1;
    default:
      return 4;
  }
};