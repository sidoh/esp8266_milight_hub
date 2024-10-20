import React, { useState, useEffect } from "react";
import { createRoot } from "react-dom/client";
import { MainNav } from "@/components/ui/main-nav";
import "./index.css";
import { Dashboard } from "./pages/dashboard";
import { NotFound } from "./pages/not-found";
import SettingsPage from "./pages/settings/settings-index";
import { Toaster } from "@/components/ui/toaster";
import { WebSocketProvider } from "@/lib/websocket";
import SnifferPage from "./pages/sniffer";
import { SettingsProvider } from "@/lib/settings";
import { LightProvider } from "@/lib/light-state";

const PAGES = {
  "/dashboard": Dashboard,
  "/not-found": NotFound,
  "/settings": SettingsPage,
  "/sniffer": SnifferPage,
};

export default function App() {
  const [currentPage, setCurrentPage] = useState<keyof typeof PAGES | null>(
    null
  );

  useEffect(() => {
    // Add dark class to body
    document.body.classList.add("dark");

    const handleHashChange = () => {
      const hash = window.location.hash.slice(1);
      setCurrentPage(hash as keyof typeof PAGES);
    };

    window.addEventListener("hashchange", handleHashChange);
    handleHashChange(); // Handle initial hash

    return () => {
      window.removeEventListener("hashchange", handleHashChange);
    };
  }, []);

  const PageComponent = currentPage
    ? PAGES[currentPage] || PAGES["/not-found"]
    : PAGES["/dashboard"];

  return (
    <LightProvider>
      <SettingsProvider>
        <WebSocketProvider>
          <div className="bg-background text-foreground flex flex-col items-center justify-start">
            <div className="container mx-auto px-4">
              <MainNav />
              <main className="flex flex-col pt-10">
                {PageComponent && <PageComponent />}
              </main>
              <Toaster />
            </div>
          </div>
        </WebSocketProvider>
      </SettingsProvider>
    </LightProvider>
  );
}

const rootElement = document.getElementById("page");
if (rootElement) {
  rootElement.innerHTML = ""; // Clear existing contents
  const root = createRoot(rootElement);
  root.render(<App />);
} else {
  console.error("Could not find element with id 'page'");
}
