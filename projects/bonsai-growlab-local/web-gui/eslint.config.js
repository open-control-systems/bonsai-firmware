import eslintPluginPrettier from "eslint-plugin-prettier";

export default [
  {
    files: ["**/*.js", "**/*.jsx"], // Specify file patterns
    languageOptions: {
      ecmaVersion: 2021,
      sourceType: "module",
      parserOptions: {
        ecmaFeatures: {
          jsx: true,
        },
      },
      globals: {
        window: "readonly",
        document: "readonly",
        console: "readonly",
        // Add other browser or ES2021 globals as needed
      },
    },
    plugins: {
      prettier: eslintPluginPrettier,
    },
    rules: {
      "prettier/prettier": "error",
    },
  },
];
