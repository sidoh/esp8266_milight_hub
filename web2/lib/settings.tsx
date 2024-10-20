import React, { createContext, useContext, useState, useEffect, ReactNode } from "react";
import { z } from "zod";
import { api, schemas } from "@/api";

type Settings = z.infer<typeof schemas.Settings>;

interface SettingsContextType {
  settings: Settings | null;
  isLoading: boolean;
  updateSettings: (newSettings: Partial<Settings>) => void;
}

const SettingsContext = createContext<SettingsContextType | null>(null);

export const SettingsProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [settings, setSettings] = useState<Settings | null>(null);
  const [isLoading, setIsLoading] = useState(true);
  useEffect(() => {
    api.getSettings().then((fetchedSettings) => {
      setSettings(fetchedSettings);
      setIsLoading(false);
    });
  }, []);

  const updateSettings = (newSettings: Partial<Settings>) => {
    const updatedSettings = { ...settings, ...newSettings };
    setSettings(updatedSettings);
    api.putSettings(updatedSettings); 
  };

  return (
    <SettingsContext.Provider value={{ settings, updateSettings, isLoading }}>
      {children}
    </SettingsContext.Provider>
  );
};

// Create a custom hook to use the SettingsContext
export const useSettings = () => {
  const context = useContext(SettingsContext);
  if (!context) {
    throw new Error("useSettings must be used within a SettingsProvider");
  }
  return context;
};

