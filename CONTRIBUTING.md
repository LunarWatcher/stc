# Contribution guidelines

This file is mostly aimed at developers, and primarily describes the setup required for development, and project-specific things to think about when contributing code. For general open-source contribution guidelines, see [opensource.guide](//opensource.guide). The guidelines listed under "Basic guidelines" do apply to all forms of contributions, including issues.

This file will not go into detail on how to write issues. Any important details that need to be included (if any) will be part of an issue template, selectable when you create an issue. If none exists for your use-case (or at all), use common sense. I do strongly suggest reading [the section on communicating effectively on opensource.guide](https://opensource.guide/how-to-contribute/#communicating-effectively) if you're wondering how to write good issues. There's nothing anyone could write here that isn't covered there and in thousands of other resources around the internet in far greater detail.

## Basic guidelines

### Use of generative AI is banned

Generative AI uses training data [based on plagiarism and piracy](https://web.archive.org/web/20250000000000*/https://www.theatlantic.com/technology/archive/2025/03/libgen-meta-openai/682093/), has [significant environmental costs associated with it](https://doi.org/10.21428/e4baedd9.9070dfe7), and [generates fundamentally insecure code](https://doi.org/10.1007/s10664-024-10590-1). GenAI is not ethically built, ethical to use, nor safe to use for programming applications. When caught, you will be permanently banned from contributing to the project, and any prior contributions will be checked and potentially reverted. Any and all contributions you've made cannot be trusted if AI slop machines were involved.

## Design/structure principles

Stc is primarily meant to be a collection of mini-libraries. As far as reasonably possible, these libraries should not depend on each other. Exceptions do apply, in which case, the exact dependencies should be minimized and documented.

Though the recommended instructions for installation are to use FetchContent, this should not be assumed. Stc is designed to be both FetchContent'd, and to be vendorable. For this goal, minimising dependencies is a requirement.

Stc started out header-only, but is shifting towards allowing source files. This is largely because `Windows.h` is a fucking cancer - if you include it in anything, and don't define the correct global macros, including `Windows.h` directly or indirectly can quietly break your build with obscure errors that are just macros. Rather than solving this like stb did (adding a macro that adds the implementation in a single source file), header-source file pairs are used. These are kept next to each other in the include tree to allow vendoring to just be open folder, select 1-2 files, copy, pasta, and register with the build system if applicable.

## Test principles

As much as reasonably possible that can be tested should be tested.

The tests are designed to be specific to stc's test setup, however, as porting tests to be equally portable and vendorable is not a goal. The structure principles do not apply to the test suite itself.

