import React, { useState } from "react";
import { useWebSocketContext } from "@/lib/websocket";
import { CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { schemas } from "@/api";
import { z } from "zod";
import { Copy } from "lucide-react";
import { useToast } from "@/hooks/use-toast";

type PacketMessage = z.infer<typeof schemas.PacketMessage>;

// Utility function to convert packet to hex string
const packetToHexString = (packet: number[]): string => {
  return packet.map(byte => byte.toString(16).padStart(2, '0').toUpperCase()).join(' ');
};

export function PacketMessageDisplay() {
  const { allMessages } = useWebSocketContext();
  const [selectedMessage, setSelectedMessage] = useState<PacketMessage | null>(null);
  const toast = useToast();

  const copyToClipboard = (text: string) => {
    navigator.clipboard.writeText(text).then(() => {
      toast.toast({
        title: "Copied to clipboard",
        description: "Device ID has been copied to clipboard",
      });
    }).catch(err => {
      console.error("Failed to copy: ", err);
    });
  };

  const renderEventList = () => {
    return [...allMessages].reverse().map((message, index) => (
      <Button
        key={index}
        variant="ghost"
        className="w-full text-left justify-start flex flex-col items-start p-2 h-auto"
        onClick={() => setSelectedMessage(message)}
      >
        <div className="flex space-x-2 mb-1">
          <Badge variant="secondary">Device ID: {message.d.di}</Badge>
          <Badge variant="secondary">Group ID: {message.d.gi}</Badge>
          <Badge variant="secondary">Remote Type: {message.d.rt}</Badge>
        </div>
        {message.u && Object.keys(message.u).length > 0 ? (
          <span className="text-sm text-muted-foreground">
            Command: {Object.keys(message.u)[0]} = {JSON.stringify(Object.values(message.u)[0])}
          </span>
        ) : (
          <span className="text-sm text-muted-foreground">No command</span>
        )}
      </Button>
    ));
  };

  const renderDetailedMessage = (message: PacketMessage) => {
    return (
      <div className="space-y-2">
        <p className="flex items-center">
          <strong>Device ID:</strong>
          <span className="ml-2">{message.d.di}</span>
          <Button
            variant="ghost"
            size="icon"
            className="h-6 w-6 ml-2"
            onClick={() => copyToClipboard(message.d.di.toString())}
          >
            <Copy className="h-4 w-4" />
          </Button>
        </p>
        <p><strong>Group ID:</strong> {message.d.gi}</p>
        <p><strong>Remote Type:</strong> {message.d.rt}</p>
        <p>
          <strong>Packet:</strong>{' '}
          <code className="bg-muted text-sm p-1 rounded">
            {packetToHexString(message.p)}
          </code>
        </p>
        <div>
          <strong>State:</strong>
          <pre className="text-xs mt-1">{JSON.stringify(message.s, null, 2)}</pre>
        </div>
        {message.u && Object.keys(message.u).length > 0 && (
          <div>
            <strong>Command:</strong>
            {Object.entries(message.u).map(([key, value]) => (
              <div key={key} className="ml-2">
                <strong>{key}:</strong> {JSON.stringify(value)}
              </div>
            ))}
          </div>
        )}
      </div>
    );
  };

  return (
    <div className="grid grid-cols-2 h-[calc(100vh-2rem)] border rounded-lg overflow-hidden">
      <div className="h-full overflow-y-auto border-r">
        <CardHeader className="border-b">
          <CardTitle>Event List</CardTitle>
        </CardHeader>
        <CardContent className="mt-5">
          {renderEventList()}
        </CardContent>
      </div>
      <div className="h-full overflow-y-auto">
        <CardHeader className="border-b">
          <CardTitle>Event Details</CardTitle>
        </CardHeader>
        <CardContent className="mt-5">
          {selectedMessage ? (
            renderDetailedMessage(selectedMessage)
          ) : (
            <p className="text-muted-foreground">Select an event to view details</p>
          )}
        </CardContent>
      </div>
    </div>
  );
}

export default function SnifferPage() {
  return (
    <div className="container mx-auto p-4">
      <PacketMessageDisplay />
    </div>
  );
}
