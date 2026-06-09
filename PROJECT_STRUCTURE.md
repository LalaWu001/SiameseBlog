# Project Structure

## Main Site

- `src/pages/`: Astro routes for the blog, notes, search, links, games, and supporting pages.
- `src/components/base/Header.astro`: Shared site header and primary navigation.
- `src/content/`: Blog posts, notes, and standalone content pages.
- `public/`: Static files copied directly into the production build.
- `src/pages/games.astro`: Online game listing linked from the main navigation.
- `public/games/watermelon-merge/`: Self-contained 合成大南开 Canvas game and badge artwork.

## Project Showcase

- `public/projects/llmtrans/index.html`: LLMtrans showcase home page.
- `public/projects/llmtrans/conversation.html`: Conversation product introduction.
- `public/projects/llmtrans/vpn.html`: VPN product introduction.
- `public/projects/llmtrans/team.html`: Team introduction.
- `public/projects/llmtrans/styles.css`: Shared responsive presentation styles.
- `public/projects/llmtrans/script.js`: Shared navigation, feature notices, mobile background-video playback recovery, reveal effects, and invitation-gated client downloads.
- `public/projects/llmtrans/IMG_5716.PNG`: Shared project icon.
- `public/projects/llmtrans/avatar/`: Team portraits used by the showcase.

The deployed showcase entry is `/projects/llmtrans/`. Its pages remain a self-contained static subsite and use relative links for internal navigation and assets.

## Build And Deployment

- `astro.config.mjs`: Astro static-site configuration.
- `package.json`: Development, build, preview, and Pagefind scripts.
- `dist/`: Generated production output; ignored by Git.
- Vercel builds the Astro site from the GitHub repository and publishes the generated static output.
