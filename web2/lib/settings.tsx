import React, { createContext, useContext, useState, useEffect, ReactNode } from "react";
import { z } from "zod";
import { api, schemas } from "@/api";

type Settings = z.infer<typeof schemas.Settings>;

interface SettingsContextType {
  settings: Settings | null;
  isLoading: boolean;
  updateSettings: (newSettings: Partial<Settings>) => void;
  theme: "light" | "dark";
  toggleTheme: () => void;
}

const SettingsContext = createContext<SettingsContextType | null>(null);

export const SettingsProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [settings, setSettings] = useState<Settings | null>(null);
  const [isLoading, setIsLoading] = useState(true);
  const [theme, setTheme] = useState<"light" | "dark">("light");

  useEffect(() => {
    api.getSettings().then((fetchedSettings) => {
      setSettings(fetchedSettings);
      setIsLoading(false);
    });

    // Initialize theme from localStorage or system preference
    const savedTheme = localStorage.getItem("theme");
    const prefersDarkMode = window.matchMedia("(prefers-color-scheme: dark)").matches;
    const initialTheme = savedTheme ? savedTheme : (prefersDarkMode ? "dark" : "light");

    setTheme(initialTheme as "light" | "dark");
  }, []);

  const updateSettings = (newSettings: Partial<Settings>) => {
    const updatedSettings = { ...settings, ...newSettings };
    setSettings(updatedSettings);
    api.putSettings(updatedSettings);
  };

  const toggleTheme = () => {
    const newTheme = theme === "dark" ? "light" : "dark";
    setTheme(newTheme);
    localStorage.setItem("theme", newTheme);
  };

  useEffect(() => {
    if (theme === "dark") {
      document.body.className = "dark";
    } else {
      document.body.className = "";
    }
  }, [theme]);

  return (
    <SettingsContext.Provider value={{ settings, updateSettings, isLoading, theme, toggleTheme }}>
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
