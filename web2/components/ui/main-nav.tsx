import * as React from "react";
import { cn } from "@/lib/utils";
import { NavLink, Link } from "@/components/ui/link";
import { Settings, Sun, Moon } from "lucide-react";
import { useSettings } from "@/lib/settings";

function ThemeSwitcher() {
  const { theme, toggleTheme } = useSettings();

  return (
    <button
      onClick={toggleTheme}
      className="text-slate-500 hover:text-slate-900 dark:text-slate-400 dark:hover:text-slate-100"
    >
      {theme === "dark" ? <Sun size={20} /> : <Moon size={20} />}
    </button>
  );
}

export function MainNav({
  className,
  ...props
}: React.HTMLAttributes<HTMLElement>) {
  const { settings, isLoading } = useSettings();

  return (
    <div className="w-full">
      <div className="flex h-16 items-center px-4 justify-between">
        <div className="flex items-center">
          <Link
            className="hover:text-slate-900 dark:hover:text-slate-100 text-slate-900 dark:text-slate-100 text-lg font-bold"
            href="#/dashboard"
          >
            {!isLoading ? `MiLight Hub: ${settings?.hostname}` : "MiLight Hub"}
          </Link>
          <nav
            className={cn(
              "flex items-center space-x-4 lg:space-x-6 mx-6",
              className
            )}
            {...props}
          >
            <NavLink href="#/dashboard">Dashboard</NavLink>
            <NavLink href="#/sniffer">Sniffer</NavLink>
          </nav>
        </div>
        <div className="flex items-center space-x-4">
          <ThemeSwitcher />
          <Link
            href="#/settings"
            className="text-slate-500 hover:text-slate-900 dark:text-slate-400 dark:hover:text-slate-100"
          >
            <Settings size={24} />
          </Link>
        </div>
      </div>
    </div>
  );
}
