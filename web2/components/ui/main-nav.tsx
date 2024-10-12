import * as React from "react"
import { cn } from "@/lib/utils"
import { NavLink, Link } from "@/components/ui/link"

export function MainNav({
    className,
    ...props
}: React.HTMLAttributes<HTMLElement>) {
    return (
        <div className="">
            <div className="flex h-16 items-center px-4">
                <Link className="hover:text-slate-900 dark:hover:text-slate-100 text-slate-900 dark:text-slate-100 text-lg font-bold" href="#/dashboard">Light Hub</Link>
                <nav
                    className={cn("flex items-center space-x-4 lg:space-x-6 mx-6", className)}
                    {...props}
                >
                    <NavLink
                        href="#/dashboard"
                    >
                        Dashboard
                    </NavLink>
                    <NavLink
                        href="#/settings"
                    >
                        Settings
                    </NavLink>
                </nav>
            </div>
        </div>
    )
}