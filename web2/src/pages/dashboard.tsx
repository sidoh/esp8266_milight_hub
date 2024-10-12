import React, { useEffect, useState } from "react";
import { LightCard } from "@/components/light/light-card";
import { GatewayListItem } from "@/api";
import { api } from "@/lib/api";
import { LightList } from "@/components/light/light-list";

export function Dashboard() {
  return (
    <div className="flex flex-col items-center justify-center">
      <LightList />
    </div>
  );
}
