"use client";

import * as React from "react";
import { cn } from "@/lib/utils";
import { Button } from "@/components/ui/button";

type NavId = string;

export interface NavItem {
  title: string;
  id: NavId;
}

interface SidebarPillNavProps extends React.HTMLAttributes<HTMLElement> {
  items: NavItem[];
}

// New type definition
export type NavChildProps<T extends NavId> =
  React.HTMLAttributes<HTMLElement> & {
    navId: T;
  };

// Update the SidebarPillNav component to use the new type
export function SidebarPillNav<T extends NavId>({
  className,
  items,
  children,
  ...props
}: React.PropsWithChildren<SidebarPillNavProps>) {
  const [activeId, setActiveId] = React.useState<NavId>(items[0]?.id || "");

  const childrenArray = React.Children.toArray(children);
  const navIds = childrenArray
    .filter(React.isValidElement)
    .map(
      (child) => (child as React.ReactElement<NavChildProps<T>>).props.navId
    );

  // Validate that all item ids are present in children's navIds
  React.useEffect(() => {
    items.forEach((item) => {
      if (!navIds.includes(item.id as T)) {
        console.warn(`Item id "${item.id}" does not match any child's navId`);
      }
    });
  }, [items, navIds]);

  return (
    <div className="container flex flex-col space-y-8 lg:flex-row lg:space-x-12 lg:space-y-0">
      <nav className="flex space-x-2 lg:flex-col lg:space-x-0 lg:space-y-1 mb-4 -mx-4 xl:w-1/5">
        {items.map((item) => (
          <Button
            key={item.id}
            variant="ghost"
            className={cn(
              activeId === item.id
                ? "bg-muted hover:bg-muted"
                : "hover:bg-transparent hover:underline",
              "justify-start"
            )}
            onClick={() => setActiveId(item.id)}
          >
            {item.title}
          </Button>
        ))}
      </nav>
      <div className="mt-4 w-full">
        {childrenArray.find(
          (child) =>
            React.isValidElement(child) &&
            (child as React.ReactElement<NavChildProps<T>>).props.navId ===
              activeId
        )}
      </div>
    </div>
  );
}
