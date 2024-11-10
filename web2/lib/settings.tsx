import React, {
  createContext,
  useContext,
  useState,
  useEffect,
  ReactNode,
  useCallback,
} from "react";
import { z } from "zod";
import { api, schemas } from "@/api";
import { useLightState } from "./light-state";

type Settings = z.infer<typeof schemas.Settings>;
type About = z.infer<typeof schemas.About>;

interface SettingsContextType {
  isLoading: boolean;
  settings: Settings | null;

  about: About | null;
  isLoadingAbout: boolean;
  updateSettings: (newSettings: Partial<Settings>) => void;
  theme: "light" | "dark";
  toggleTheme: () => void;
  reloadAbout: () => void;
}

const SettingsContext = createContext<SettingsContextType | null>(null);

export const SettingsProvider: React.FC<{ children: ReactNode }> = ({
  children,
}) => {
  const { lightStates } = useLightState();
  const [settings, setSettings] = useState<Settings | null>(null);
  const [about, setAbout] = useState<About | null>(null);
  const [isLoading, setIsLoading] = useState(true);
  const [isLoadingAbout, setIsLoadingAbout] = useState(true);
  const [theme, setTheme] = useState<"light" | "dark">("light");

  const reloadAbout = useCallback(() => {
    setIsLoadingAbout(true);
    api
      .getAbout()
      .then(setAbout)
      .finally(() => setIsLoadingAbout(false));
  }, []);

  useEffect(() => {
    // Initialize theme from localStorage or system preference
    const savedTheme = localStorage.getItem("theme");
    const prefersDarkMode = window.matchMedia(
      "(prefers-color-scheme: dark)"
    ).matches;
    const initialTheme = savedTheme
      ? savedTheme
      : prefersDarkMode
      ? "dark"
      : "light";

    setTheme(initialTheme as "light" | "dark");
  }, []);

  useEffect(() => {
    const fetchData = async () => {
      const [settings, about] = await Promise.all([
        api.getSettings(),
        api.getAbout(),
      ]);
      setSettings(settings);
      setAbout(about);
      setIsLoading(false);
      setIsLoadingAbout(false);
    };

    // Force serial fetching of /gateways => /settings => /about
    if (!lightStates.isLoading && isLoading) {
      fetchData();
    }
  }, [lightStates.isLoading, isLoading]);

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
    <SettingsContext.Provider
      value={{
        settings,
        about,
        updateSettings,
        isLoading,
        isLoadingAbout,
        theme,
        toggleTheme,
        reloadAbout,
      }}
    >
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
