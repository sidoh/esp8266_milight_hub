import * as React from "react";
import { Controller, useFormContext } from "react-hook-form";
import { z } from "zod";
import { Input } from "@/components/ui/input";
import { schemas } from "@/api";
import {
  FormField,
  FormItem,
  FormLabel,
  FormControl,
  FormDescription,
  FormMessage,
} from "@/components/ui/form";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import { ToggleGroup, ToggleGroupItem } from "@/components/ui/toggle-group";

export const extractSchemaType = (schema: z.ZodTypeAny): z.ZodTypeAny => {
  if (schema instanceof z.ZodOptional || schema instanceof z.ZodNullable) {
    return extractSchemaType(schema.unwrap());
  } else if (schema instanceof z.ZodDefault) {
    return extractSchemaType(schema.removeDefault());
  } else if (schema instanceof z.ZodUnion) {
    // don't try to be fancy, just pick an arbitrary option
    return extractSchemaType(schema.options[0]);
  }
  return schema;
};

export type Settings = z.infer<typeof schemas.Settings>;
export type SettingsKey = keyof typeof schemas.Settings.shape;

export const DynamicFormControl: React.FC<{
  field: keyof typeof schemas.Settings.shape;
  fieldType?: "text" | "number" | "password";
}> = ({ field, fieldType: fieldTypeOverride }) => {
  const form = useFormContext<Settings>();
  const fieldSchema = schemas.Settings.shape[field];
  const fieldType = extractSchemaType(fieldSchema);

  if (fieldType instanceof z.ZodString || fieldType instanceof z.ZodNumber) {
    let inputType = fieldTypeOverride || "text";
    if (fieldTypeOverride) {
      inputType = fieldTypeOverride;
    } else if (fieldType instanceof z.ZodString) {
      inputType = "text";
    } else if (fieldType instanceof z.ZodNumber) {
      inputType = "number";
    }

    return (
      <Controller
        control={form.control}
        name={field}
        render={({ field: formField }) => (
          <Input
            type={inputType}
            {...formField}
            value={formField.value as string | number | undefined}
            onChange={(e) =>
              inputType === "number"
                ? formField.onChange(
                    Number.isNaN(e.target.valueAsNumber)
                      ? e.target.value
                      : e.target.valueAsNumber
                  )
                : formField.onChange(e.target.value)
            }
          />
        )}
      />
    );
  } else if (fieldType instanceof z.ZodEnum) {
    const numOptions = fieldType.options.length;

    if (numOptions <= 4) {
      return (
        <Controller
          control={form.control}
          name={field}
          render={({ field: formField }) => (
            <ToggleGroup
              type="single"
              variant="outline"
              onValueChange={(value) => {
                formField.onChange(value);
              }}
              onBlur={() => {}}
              value={formField.value as string}
            >
              {fieldType.options.map((option: string) => (
                <ToggleGroupItem key={option} value={option}>
                  {option}
                </ToggleGroupItem>
              ))}
            </ToggleGroup>
          )}
        />
      );
    } else {
      return (
        <Controller
          control={form.control}
          name={field}
          render={({ field: formField }) => (
            <Select
              onValueChange={(value) => {
                formField.onChange(value);
                formField.onBlur();
              }}
              value={formField.value as string}
            >
              <FormControl>
                <SelectTrigger>
                  <SelectValue placeholder="Select an option" />
                </SelectTrigger>
              </FormControl>
              <SelectContent>
                {fieldType.options.map((option: string) => (
                  <SelectItem key={option} value={option} className="group">
                    <div className="flex flex-col items-start max-w-72">
                      <div className="font-medium">{option}</div>
                    </div>
                  </SelectItem>
                ))}
              </SelectContent>
            </Select>
          )}
        />
      );
    }
  } else if (fieldType instanceof z.ZodBoolean) {
    return (
      <Controller
        control={form.control}
        name={field}
        render={({ field: formField }) => (
          <ToggleGroup
            type="single"
            variant="outline"
            onValueChange={(value) => {
              formField.onChange(value === "true");
              formField.onBlur();
            }}
            value={formField.value ? "true" : "false"}
          >
            <ToggleGroupItem value="true">Enabled</ToggleGroupItem>
            <ToggleGroupItem value="false">Disabled</ToggleGroupItem>
          </ToggleGroup>
        )}
      />
    );
  } else if (fieldType instanceof z.ZodArray) {
    const itemType = extractSchemaType(fieldType.element);
    if (itemType instanceof z.ZodEnum) {
      return (
        <Controller
          control={form.control}
          name={field}
          render={({ field: formField }) => (
            <ToggleGroup
              type="multiple"
              variant="outline"
              onValueChange={(values) => {
                formField.onChange(values);
                formField.onBlur();
              }}
              value={formField.value as string[]}
            >
              {itemType.options.map((option: string) => (
                <ToggleGroupItem key={option} value={option}>
                  {option}
                </ToggleGroupItem>
              ))}
            </ToggleGroup>
          )}
        />
      );
    }
  } else {
    return <></>;
  }
};

export const StandardFormField: React.FC<{
  field: SettingsKey;
  nameOverride?: string;
  children: React.ReactNode;
  className?: string;
}> = ({ field, nameOverride, children, className }) => {
  const form = useFormContext<Settings>();
  const fieldSchema = schemas.Settings.shape[field];
  const fieldState = form.getFieldState(field);

  return (
    <FormField
      key={field}
      control={form.control}
      name={field}
      render={({ field: formField }) => (
        <FormItem className={className}>
          <FormLabel className="flex items-center h-8">
            <span>
              {nameOverride ||
                field
                  .replace(/_/g, " ")
                  .replace(/\b\w/g, (l) => l.toUpperCase())}
            </span>
            {fieldState.isDirty && (
              <span className="text-lg text-muted-foreground ml-1">*</span>
            )}
          </FormLabel>
          <FormControl>{children}</FormControl>
          <FormDescription>{fieldSchema.description}</FormDescription>
          <FormMessage />
        </FormItem>
      )}
    />
  );
};

export const FormFields: React.FC<{
  fields: SettingsKey[];
  fieldNames?: Partial<Record<SettingsKey, string>>;
  fieldTypes?: Partial<Record<SettingsKey, "text" | "number" | "password">>;
}> = ({ fields, fieldNames, fieldTypes }) => {
  const form = useFormContext<Settings>();

  return (
    <div className="space-y-4">
      {fields.map((field) => {
        return (
          <FormField
            key={field}
            control={form.control}
            name={field}
            render={({ field: formField }) => (
              <StandardFormField
                field={field}
                nameOverride={fieldNames?.[field]}
              >
                <DynamicFormControl
                  field={field}
                  fieldType={fieldTypes?.[field]}
                />
              </StandardFormField>
            )}
          />
        );
      })}
    </div>
  );
};

export const FieldSection: React.FC<{
  title: string;
  description?: string;
  fields: (keyof typeof schemas.Settings.shape)[];
  fieldNames?: Partial<Record<SettingsKey, string>>;
  fieldTypes?: Partial<Record<SettingsKey, "text" | "number" | "password">>;
  children?: React.ReactNode;
}> = ({ title, description, fields, fieldNames, fieldTypes, children }) => (
  <div>
    <h2 className="text-2xl font-bold">{title}</h2>
    {description && <p className="text-sm text-gray-500">{description}</p>}
    <hr className="my-4" />
    <FormFields
      fields={fields}
      fieldNames={fieldNames}
      fieldTypes={fieldTypes}
    />
    {children}
  </div>
);

export const FieldSections: React.FC<{ children: React.ReactNode }> = ({
  children,
}) => <div className="flex flex-col space-y-10 max-w-xl">{children}</div>;
