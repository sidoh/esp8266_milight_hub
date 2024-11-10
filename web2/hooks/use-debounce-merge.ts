import { useState, useEffect, useRef } from 'react';

export function useDebounceMerge<T>(initialValue: T, delay: number): [T, (newValue: T | ((prevValue: T) => T)) => void] {
  const [value, setValue] = useState<T>(initialValue);
  const timeoutRef = useRef<NodeJS.Timeout | null>(null);

  useEffect(() => {
    return () => {
      if (timeoutRef.current) {
        clearTimeout(timeoutRef.current);
      }
    };
  }, []);

  const debouncedSetValue = (newValue: T | ((prevValue: T) => T)) => {
    if (timeoutRef.current) {
      clearTimeout(timeoutRef.current);
    }

    timeoutRef.current = setTimeout(() => {
      setValue((prevValue) => {
        if (typeof newValue === 'function') {
          return (newValue as (prevValue: T) => T)(prevValue);
        }
        if (typeof prevValue === 'object' && typeof newValue === 'object') {
          return { ...prevValue, ...newValue };
        }
        return newValue;
      });
    }, delay);
  };

  return [value, debouncedSetValue];
}