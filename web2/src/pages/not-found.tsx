import React from "react";

export function NotFound() {
  return (
    <div className="flex flex-col items-center justify-center h-full bg-background text-foreground">
      <h1 className="text-6xl font-bold mb-4">404</h1>
      <p className="text-xl mb-8">Page Not Found</p>
      <a href="#/dashboard" className="text-primary hover:underline">
        Go back to Dashboard
      </a>
    </div>
  );
}
