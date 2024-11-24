import React from "react";
import { LightList } from "@/components/light/light-list";
import { ManualControlCard } from "@/components/light/manual-control-card";

export function Dashboard() {
  return (
    <div className="flex flex-row space-x-4 items-baseline justify-center">
      <LightList />
      <ManualControlCard />
    </div>
  );
}
