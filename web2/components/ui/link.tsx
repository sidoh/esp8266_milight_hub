import React, { useEffect, useState } from 'react';
import { cn } from '@/lib/utils';

interface LinkProps extends React.AnchorHTMLAttributes<HTMLAnchorElement> {
    href: string;
    className?: string;
}

const Link = React.forwardRef<HTMLAnchorElement, LinkProps>(
    ({ href, className, ...props }, ref) => {
        return (
            <a
                ref={ref}
                href={href}
                className={cn(
                    "hover:border-slate-400 dark:hover:border-slate-500 text-slate-700 hover:text-slate-900 dark:text-slate-400 dark:hover:text-slate-300",
                    className
                )}
                {...props}
            />
        );
    }
);

Link.displayName = 'Link';

const NavLink = React.forwardRef<HTMLAnchorElement, LinkProps>(
    ({ href, className, ...props }, ref) => {
        const [isActive, setIsActive] = useState(false);

        useEffect(() => {
            const checkIfActive = () => {
                setIsActive(window.location.hash === href);
            };

            checkIfActive();
            window.addEventListener('popstate', checkIfActive);

            return () => {
                window.removeEventListener('popstate', checkIfActive);
            };
        }, [href]);

        return (
            <Link
                ref={ref}
                href={href}
                className={cn(
                    { "underline decoration-2 underline-offset-8": isActive },
                    className
                )}
                {...props}
            />
        );
    }
);

NavLink.displayName = 'NavLink';

export { Link, NavLink };

