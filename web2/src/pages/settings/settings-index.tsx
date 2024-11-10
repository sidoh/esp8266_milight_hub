import * as React from "react";
import { FormProvider, useForm } from "react-hook-form";
import { zodResolver } from "@hookform/resolvers/zod";
import { z } from "zod";
import {
  NavChildProps,
  NavItem,
  SidebarPillNav,
} from "@/components/ui/sidebar-pill-nav";
import { api, schemas } from "@/api";
import { useEffect, useState, useCallback } from "react";
import { Skeleton } from "@/components/ui/skeleton";
import { extractSchemaType, SettingsKey } from "./form-components";
import { MQTTSettings } from "./section-mqtt";
import { HardwareSettings } from "./section-hardware";
import { NetworkSettings } from "./section-network";
import { SystemSettings } from "./section-system";
import { RadioSettings } from "./section-radio";
import { StateSettings } from "./section-state";
import { UDPSettings } from "./section-udp";
import { debounce } from "lodash";
import { useSettings } from "@/lib/settings";

type Settings = z.infer<typeof schemas.Settings>;

const settingsNavItems: NavItem[] = [
  { title: "Network", id: "network" },
  { title: "Hardware", id: "hardware" },
  { title: "MQTT", id: "mqtt" },
  { title: "Radio", id: "radio" },
  { title: "State", id: "state" },
  { title: "UDP", id: "udp" },
  { title: "System", id: "system" },
];

export default function SettingsPage() {
  const { settings, isLoading } = useSettings();

  const form = useForm<Settings>({
    resolver: zodResolver(schemas.Settings),
    defaultValues: {} as Settings,
    mode: "onBlur",
  });

  const debouncedOnSubmit = useCallback(
    debounce(() => {
      const update: Partial<Settings> = {};

      for (const field in form.formState.dirtyFields) {
        update[field as keyof Settings] = form.getValues(field);
      }

      if (Object.keys(update).length > 0) {
        api.putSettings(update).then(() => {
          form.reset(form.getValues());
        });
      }
    }, 300),
    [form]
  );

  useEffect(() => {
    const subscription = form.watch((value, { name }) => {
      if (!name || !(name in schemas.Settings.shape)) {
        return;
      }

      debouncedOnSubmit();
    });
    return () => subscription.unsubscribe();
  }, [form]);

  useEffect(() => {
    if (settings) {
      form.reset(settings);
    }
  }, [settings]);

  return isLoading ? (
    <div className="flex justify-center h-screen space-x-4">
      <div className="w-1/5 h-full max-h-96">
        <Skeleton className="w-full h-full" />
      </div>
      <div className="w-3/5 h-full flex flex-col space-y-4">
        <Skeleton className="w-full h-10" />
        <Skeleton className="w-full h-10" />
        <Skeleton className="w-full h-10" />
      </div>
    </div>
  ) : (
    <FormProvider {...form}>
      <form
        onBlur={debouncedOnSubmit}
        onSubmit={(e) => {
          e.preventDefault();
          form.handleSubmit(debouncedOnSubmit)();
        }}
      >
        <SidebarPillNav items={settingsNavItems}>
          <NetworkSettings navId="network" />
          <HardwareSettings navId="hardware" />
          <MQTTSettings navId="mqtt" />
          <RadioSettings navId="radio" />
          <StateSettings navId="state" />
          <UDPSettings navId="udp" />
          <SystemSettings navId="system" />
        </SidebarPillNav>
      </form>
    </FormProvider>
  );
}
