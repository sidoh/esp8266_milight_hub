import { useRef } from "react";
import { useRateLimitMerge } from "@/hooks/use-rate-limit-merge";
import { api } from "@/lib/api";
import { z } from "zod";
import { schemas } from "@/api/api-zod";

export function useUpdateState(
  id: z.infer<typeof schemas.BulbId>,
  _updateState: (payload: Partial<z.infer<typeof schemas.NormalizedGroupState>>) => void
) {
  const [rateLimitedState, setRateLimitedState, clearRateLimitedState] =
    useRateLimitMerge<
      z.infer<typeof schemas.putGatewaysDeviceIdRemoteTypeGroupId_Body>
    >({}, 500);
  const lastUpdateTimeRef = useRef<number>(0);

  const sendUpdate = async (
    state: z.infer<typeof schemas.putGatewaysDeviceIdRemoteTypeGroupId_Body>
  ) => {
    const response = await api.putGatewaysDeviceIdRemoteTypeGroupId(state, {
      params: {
        remoteType: id.device_type,
        deviceId: id.device_id,
        groupId: id.group_id,
      },
      queries: {
        fmt: "normalized",
        blockOnQueue: true,
      },
    });
    if (response) {
      _updateState(
        response as Partial<z.infer<typeof schemas.NormalizedGroupState>>
      );
    }
  };

  const updateState = (
    newState: Partial<z.infer<typeof schemas.NormalizedGroupState>>
  ) => {
    _updateState(newState);
    const now = Date.now();
    if (now - lastUpdateTimeRef.current >= 500) {
      sendUpdate(newState);
      lastUpdateTimeRef.current = now;
      clearRateLimitedState();
    } else {
      setRateLimitedState((prevState) => ({ ...prevState, ...newState }));
    }
  };

  return {
    updateState,
    rateLimitedState,
    sendUpdate,
    clearRateLimitedState
  };
} 