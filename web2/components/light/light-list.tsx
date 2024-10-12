import React, { useReducer, useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Switch } from "@/components/ui/switch";
import { api } from "@/lib/api";
import { GatewayListItem, NormalizedGroupState } from "@/api";
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from "@/components/ui/dialog";
import { reducer } from "./state"; // Import the reducer
import { LightStatusIcon } from "./light-status-icon";
import { Plus, Pencil, Trash } from "lucide-react"; // Import the Plus, Pencil, Trash, and X icons
import ConfirmationDialog from "@/components/confirmation-dialog"; // Import the ConfirmationDialog
import NewLightForm, { NewAlias } from "./new-light-form";
import { LightCard } from "./light-card"; // Import the LightCard component
import { cn } from "@/lib/utils"; // Make sure to import the cn utility if not already present
import { Skeleton } from "../ui/skeleton";

interface LightListProps {
  lights: GatewayListItem[];
}
export function LightList() {
  const [lightStates, dispatch] = useReducer(reducer, {
    lights: [],
    isLoading: true,
  });
  const [isDeleteMode, setIsDeleteMode] = useState(false); // State to track delete mode
  const [showConfirmation, setShowConfirmation] = useState(false); // State to manage dialog visibility
  const [lightToDelete, setLightToDelete] = useState<GatewayListItem | null>(
    null
  ); // State to track which light to delete
  const [selectedLightId, setSelectedLightId] = useState<number | null>(null); // State for selected light

  useEffect(() => {
    const loadInitialState = async () => {
      const response = await api.deviceControl.gatewaysGet();
      dispatch({ type: "SET_LIGHTS", lights: response.data });
    };
    loadInitialState();
  }, []);

  const updateGroup = (light: GatewayListItem, state: NormalizedGroupState) => {
    console.log("updateGroup", state);
    dispatch({
      type: "UPDATE_STATE",
      device: light.device,
      payload: state,
    });
  };

  const handleSwitchChange = (light: GatewayListItem, checked: boolean) => {
    const update: NormalizedGroupState = {
      state: checked ? "ON" : ("OFF" as "ON" | "OFF"),
    };
    updateGroup(light, update);
    api.deviceControl
      .gatewaysDeviceIdRemoteTypeGroupIdPut(
        light.device.device_id,
        light.device.device_type,
        light.device.group_id,
        false,
        "normalized",
        update
      )
      .then((response) => {
        console.log(response);
      });
  };

  const handleAddLight = async (data: NewAlias) => {
    console.log(data);
    api.aliases.aliasesPost(data).then((response) => {
      dispatch({
        type: "ADD_LIGHT",
        device: { ...data, id: response.data.id! },
      });
    });
  };

  const handleAddButtonClick = () => {
    // Stub for add button click event
    console.log("Add button clicked");
  };

  const handleDeleteButtonClick = (light: GatewayListItem) => {
    setLightToDelete(light);
    setShowConfirmation(true);
  };

  const confirmDelete = async () => {
    if (lightToDelete) {
      await api.aliases.aliasesIdDelete(lightToDelete.device.id);
      dispatch({ type: "DELETE_LIGHT", device: lightToDelete.device });
      setLightToDelete(null);
    }
    setShowConfirmation(false);
  };

  const cancelDelete = () => {
    setLightToDelete(null);
    setShowConfirmation(false);
  };

  const handleLightClick = (light: GatewayListItem) => {
    setSelectedLightId(light.device.id);
    console.log(light);
  };

  const onNameChange = (light: GatewayListItem, newName: string) => {
    api.aliases.aliasesIdPut(light.device.id, { alias: newName });
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
            <Dialog>
              <DialogTrigger>
                <button
                  className="text-gray-500 hover:text-gray-700"
                  onClick={handleAddButtonClick}
                  aria-label="Add new light"
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
