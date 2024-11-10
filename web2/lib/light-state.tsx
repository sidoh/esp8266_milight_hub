import React, { createContext, useContext, useReducer, useEffect, ReactNode } from "react";
import { z } from "zod";
import { api, schemas } from "@/api";
import { reducer } from "@/components/light/state";

type LightState = z.infer<typeof schemas.GatewayListItem>;

interface LightContextType {
  lightStates: { lights: LightState[]; isLoading: boolean };
  dispatch: React.Dispatch<any>;
}

const LightContext = createContext<LightContextType | null>(null);

export const LightProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [lightStates, dispatch] = useReducer(reducer, {
    lights: [],
    isLoading: true,
  });

  useEffect(() => {
    const loadInitialState = async () => {
      const response = await api.getGateways();
      dispatch({ type: "SET_LIGHTS", lights: response });
    };
    loadInitialState();
  }, []);

  return (
    <LightContext.Provider value={{ lightStates, dispatch }}>
      {children}
    </LightContext.Provider>
  );
};

export const useLightState = () => {
  const context = useContext(LightContext);
  if (!context) {
    throw new Error("useLightState must be used within a LightProvider");
  }
  return context;
};
