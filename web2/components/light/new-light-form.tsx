import React from "react";
import { zodResolver } from "@hookform/resolvers/zod";
import { useForm } from "react-hook-form";
import { z } from "zod";

import { Button } from "@/components/ui/button";
import {
  Form,
  FormControl,
  FormField,
  FormItem,
  FormLabel,
  FormMessage,
} from "@/components/ui/form";
import { Input } from "@/components/ui/input";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select"; // Assuming you have a Select component
import { Alias, RemoteType } from "@/api";

const schema = z.object({
  name: z.string().min(1, { message: "Name is required." }),
  device_type: z.nativeEnum(RemoteType),
  device_id: z
    .string()
    .regex(/^(0x[0-9A-Fa-f]+|[0-9]+)$/, {
      message:
        "Invalid device ID format. It should be a hexadecimal number starting with 0x or a decimal number.",
    }),
  group_id: z.number().int().min(0).max(8),
});

export interface NewAlias extends Omit<Alias, 'id'> {}

interface NewLightFormProps {
  onSubmit: (data: NewAlias) => void;
}

export function NewLightForm({ onSubmit }: NewLightFormProps) {
  const form = useForm<z.infer<typeof schema>>({
    resolver: zodResolver(schema),
  });

  const handleSubmit = (data: z.infer<typeof schema>) => {
    const parsedDeviceId = data.device_id.startsWith("0x")
      ? parseInt(data.device_id, 16)
      : parseInt(data.device_id, 10);

    const parsedData = {
      ...data,
      alias: data.name,
      device_id: parsedDeviceId,
    };

    onSubmit(parsedData);
  };

  return (
    <Form {...form}>
      <form onSubmit={form.handleSubmit(handleSubmit)} className="space-y-8">
        <FormField
          control={form.control}
          name="name"
          render={({ field }) => (
            <FormItem>
              <FormLabel>Name</FormLabel>
              <FormControl>
                <Input placeholder="Enter name" {...field} />
              </FormControl>
              <FormMessage />
            </FormItem>
          )}
        />

        <FormField
          control={form.control}
          name="device_type"
          render={({ field }) => (
            <FormItem>
              <FormLabel>Remote Type</FormLabel>
              <Select onValueChange={field.onChange} defaultValue={field.value}>
                <FormControl>
                  <SelectTrigger>
                    <SelectValue placeholder="Select a remote type" />
                  </SelectTrigger>
                </FormControl>
                <SelectContent>
                  {Object.values(RemoteType).map((type) => (
                    <SelectItem key={type} value={type}>
                      {type}
                    </SelectItem>
                  ))}
                </SelectContent>
              </Select>
              <FormMessage />
            </FormItem>
          )}
        />

        <FormField
          control={form.control}
          name="device_id"
          render={({ field }) => (
            <FormItem>
              <FormLabel>Device ID</FormLabel>
              <FormControl>
                <Input
                  type="text" // Keep input type as text to allow hex input
                  placeholder="Enter device ID"
                  {...field}
                />
              </FormControl>
              <FormMessage />
            </FormItem>
          )}
        />

        <FormField
          control={form.control}
          name="group_id"
          render={({ field }) => (
            <FormItem>
              <FormLabel>Group ID</FormLabel>
              <FormControl>
                <Input
                  type="number"
                  placeholder="Enter group ID"
                  {...field}
                  onChange={(e) => field.onChange(+e.target.value)}
                />
              </FormControl>
              <FormMessage />
            </FormItem>
          )}
        />

        <Button type="submit">Submit</Button>
      </form>
    </Form>
  );
}

export default NewLightForm;
