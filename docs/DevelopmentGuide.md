# Development Guide

[Documentation index](README.md)

- [Repository setup](#repository-setup)
- [Systematic-aware corrections](#systematic-aware-corrections)
- [Typed constants and identifiers](#typed-constants-and-identifiers)
- [Testing another pull request](#testing-another-pull-request)
- [GitLab authentication in GitHub Actions](#gitlab-authentication-in-github-actions)

## Repository setup

Start development from an up-to-date branch with a clean working tree:

```bash
git clone --recurse-submodules git@github.com:$GITACCOUNT/SKNanoAnalyzer.git
git remote add upstream git@github.com:CMSSNU/SKNanoAnalyzer.git
git fetch upstream
git rebase upstream/main
git checkout -b $DEVBRANCH
```

## Systematic-aware corrections

Corrections managed by `SystematicHelper` should end their parameter list with
the systematic variation and source:

```cpp
// Correction.h
float Correction::YOUR_NEW_FUNCTION(PARAMETERS..., const variation syst,
                                      const TString &source = "total");
```

`variation` is the scoped enumeration defined in `Correction.h`:

```cpp
enum class variation { nom, up, down };
```

Use `"total"` for the combined uncertainty and as the default source. Refer to
`Correction::GetBTaggingSF` for an existing implementation.

## Typed constants and identifiers

Prefer `enum class` values to ambiguous string keys. Electron IDs, for example,
are defined in [`LeptonIDEnums.h`](../DataFormats/include/LeptonIDEnums.h):

```cpp
enum class ElectronID {
    NOCUT,
    POG_VETO,
    POG_LOOSE,
    POG_MEDIUM,
    POG_TIGHT,
    POG_HEEP,
    POG_MVAISO_WP80,
    POG_MVAISO_WP90,
    POG_MVANOISO_WP80,
    POG_MVANOISO_WP90,
};
```

Use the constants in `AnalyzerTools/include/PhysicalConstants.h` instead of
embedding numeric literals. `AnalyzerCore.h` already includes this header:

```cpp
// Veto Z mass window
if (std::abs(mass - Z_MASS) < 15.)
    return false;
```

## Testing another pull request

Fetch a pull request into a local branch before building and testing it:

```bash
git fetch upstream pull/$PRNUMBER/head:pr$PRNUMBER
git checkout pr$PRNUMBER

# Example: PR 16
git fetch upstream pull/16/head:pr16
git checkout pr16
```

## GitLab authentication in GitHub Actions

The upstream GitHub Actions workflow needs an SSH key to access dependencies
on `gitlab.cern.ch`:

1. Create a dedicated key:

```bash
ssh-keygen -t ed25519 -C "gitlab-ci-key" -f ~/.ssh/id_ed25519_gitlab_gha
```

2. Add the public key under GitLab **Preferences > SSH Keys**.
3. Add the private key to the upstream GitHub repository as the Actions secret
   `SSH_PRIVATE_KEY`.

The `pull_request_target` workflow runs in the upstream repository context, so
adding this secret to a fork does not configure the upstream job.
