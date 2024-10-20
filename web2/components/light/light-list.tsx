import React, { useReducer, useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Switch } from "@/components/ui/switch";
import { schemas } from "@/api/api-zod";
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from "@/components/ui/dialog";
import { reducer } from "./state"; 
import { LightStatusIcon } from "./light-status-icon";
import { Plus, Pencil, Trash } from "lucide-react"; 
import ConfirmationDialog from "@/components/confirmation-dialog"; 
import NewLightForm from "./new-light-form";
import { LightCard } from "./light-card"; 
import { cn } from "@/lib/utils"; 
import { Skeleton } from "../ui/skeleton";
import { api } from "@/lib/api";
import { z } from "zod";
import { useToast } from "@/hooks/use-toast";
import { useWebSocketContext } from "@/lib/websocket";
import { useLightState } from "@/lib/light-state";

export function LightList() {
  const { lightStates, dispatch } = useLightState();
  const { lastMessage } = useWebSocketContext();
  const [isDeleteMode, setIsDeleteMode] = useState(false); 
  const [showConfirmation, setShowConfirmation] = useState(false); 
  const [lightToDelete, setLightToDelete] = useState<z.infer<
    typeof schemas.GatewayListItem
  > | null>(null); 
  const [selectedLightId, setSelectedLightId] = useState<number | null>(null); 
  const [isAddLightDialogOpen, setIsAddLightDialogOpen] = useState(false);
  const toast = useToast();

  useEffect(() => {
    if (lastMessage && lastMessage.t == "packet") {
      dispatch({
        type: "UPDATE_STATE",
        device: {
          device_id: lastMessage.d.di,
          group_id: lastMessage.d.gi,
          device_type: lastMessage.d.rt,
        },
        payload: lastMessage.s,
      });
    }
  }, [lastMessage]);

  const updateGroup = (
    light: z.infer<typeof schemas.GatewayListItem>,
    state: z.infer<typeof schemas.NormalizedGroupState>
  ) => {
    dispatch({
      type: "UPDATE_STATE",
      device: light.device,
      payload: state,
    });
  };

  const handleSwitchChange = (
    light: z.infer<typeof schemas.GatewayListItem>,
    checked: boolean
  ) => {
    const update: z.infer<typeof schemas.NormalizedGroupState> = {
      state: checked ? "ON" : ("OFF" as "ON" | "OFF"),
    };
    updateGroup(light, update);
    api.putGatewaysDeviceIdRemoteTypeGroupId(update, {
      params: {
        remoteType: light.device.device_type,
        deviceId: light.device.device_id,
        groupId: light.device.group_id,
      },
      queries: {
        fmt: "normalized",
      },
    });
  };

  const handleAddLight = async (data: z.infer<typeof schemas.Alias>) => {
    try {
      const response = await api.postAliases(data);
      dispatch({
        type: "ADD_LIGHT",
        device: { ...data, id: response.id! },
      });
      setIsAddLightDialogOpen(false); 
    } catch (error) {
      toast.toast({
        title: "Error adding light",
        description: "Please try again",
        variant: "destructive",
      });
    }
  };

  const handleDeleteButtonClick = (
    light: z.infer<typeof schemas.GatewayListItem>
  ) => {
    setLightToDelete(light);
    setShowConfirmation(true);
  };

  const confirmDelete = async () => {
    if (lightToDelete) {
      await api.deleteAliasesId(undefined, {
        params: { id: lightToDelete.device.id },
      });
      dispatch({ type: "DELETE_LIGHT", device: lightToDelete.device });
      setLightToDelete(null);
    }
    setShowConfirmation(false);
  };

  const cancelDelete = () => {
    setLightToDelete(null);
    setShowConfirmation(false);
  };

  const handleLightClick = (light: z.infer<typeof schemas.GatewayListItem>) => {
    setSelectedLightId(light.device.id);
  };

  const onNameChange = (
    light: z.infer<typeof schemas.GatewayListItem>,
    newName: string
  ) => {
    api.putAliasesId(
      { alias: newName },
      {
        params: { id: light.device.id },
      }
    );
    dispatch({
      type: "UPDATE_LIGHT_NAME",
      device: light.device,
      name: newName,
    });
  };

  return (
    <div className="flex items-center justify-center mt-10">
      <Card className="w-96">
        <CardHeader>
          <CardTitle className="text-lg font-medium">Lights</CardTitle>
        </CardHeader>
        <CardContent>
          {lightStates.isLoading ? (
            <div className="flex justify-center items-center h-24">
              <div className="space-y-4">
                <Skeleton className="h-4 w-[250px]" />
                <Skeleton className="ml-2 h-4 w-[250px]" />
                <Skeleton className="h-4 w-[250px]" />
              </div>
            </div>
          ) : (
            lightStates.lights.map((light, index) => (
              <div
                key={index}
                className="flex items-center justify-between py-2 cursor-pointer"
                onClick={() => handleLightClick(light)}
              >
                <div className="flex items-center">
                  {isDeleteMode && (
                    <button
                      className={cn(
                        "text-red-500 hover:text-red-700 mr-2",
                        "transition-transform duration-300 ease-in-out",
                        "transform scale-100"
                      )}
                      onClick={(e) => {
                        e.stopPropagation();
                        handleDeleteButtonClick(light);
                      }}
                      aria-label={`Delete ${light.device.alias}`}
                    >
                      <Trash size={16} />
                    </button>
                  )}
                  <div className="mr-2">
                    <LightStatusIcon state={light.state} />
                  </div>
                  <span>{light.device.alias}</span>
                </div>
                <Switch
                  checked={light.state.state === "ON"}
                  onClick={(e) => {
                    e.stopPropagation();
                  }}
                  onCheckedChange={(checked) => {
                    handleSwitchChange(light, checked);
                  }}
                  aria-label={`Toggle ${light.device.alias}`}
                />
              </div>
            ))
          )}
          <div className="flex justify-end mt-4">
            <button
              className={cn(
                "text-gray-500 hover:text-gray-700 mr-2",
                "transition-all duration-300 ease-in-out",
                { "rotate-180": isDeleteMode }
              )}
              onClick={() => setIsDeleteMode(!isDeleteMode)}
              aria-label="Toggle delete mode"
            >
              <Pencil size={16} />
            </button>
            <Dialog open={isAddLightDialogOpen} onOpenChange={setIsAddLightDialogOpen}>
              <DialogTrigger asChild>
                <button
                  className="text-gray-500 hover:text-gray-700"
                  aria-label="Add new light"
                  onClick={() => setIsAddLightDialogOpen(true)}
                >
                  <Plus size={24} />
                </button>
              </DialogTrigger>
              <DialogContent className="w-1/2 min-w-96 max-w-2xl">
                <DialogHeader>
                  <DialogTitle className="mb-4">Add new light</DialogTitle>
                </DialogHeader>
                <NewLightForm onSubmit={handleAddLight} />
              </DialogContent>
            </Dialog>
          </div>
        </CardContent>
      </Card>
      {showConfirmation && (
        <ConfirmationDialog
          open={showConfirmation}
          setOpen={setShowConfirmation}
          onConfirm={confirmDelete}
          onCancel={cancelDelete}
          title="Confirm Deletion"
          description={`Are you sure you want to delete ${lightToDelete?.device.alias}?`}
        />
      )}
      {selectedLightId && (
        <Dialog
          open={!!selectedLightId}
          onOpenChange={() => setSelectedLightId(null)}
        >
          <DialogContent
            className="p-0 border-none bg-transparent"
            closeButton={false}
          >
            {(() => {
              const selectedLight = lightStates.lights.find(
                (light) => light.device.id === selectedLightId
              );
              return (
                selectedLight && (
                  <LightCard
                    name={selectedLight.device.alias}
                    state={selectedLight.state}
                    id={selectedLight.device}
                    updateState={(payload) => {
                      updateGroup(selectedLight, payload);
                    }}
                    onClose={() => setSelectedLightId(null)}
                    onNameChange={(newName) => {
                      onNameChange(selectedLight, newName);
                    }}
                  />
                )
              );
            })()}
          </DialogContent>
        </Dialog>
      )}
    </div>
  );
}
