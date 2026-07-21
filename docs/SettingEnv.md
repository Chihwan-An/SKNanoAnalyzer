# Setting up micromamba + singularity environment

[Documentation index](README.md)
 
## Overview

This guide sets up the shared micromamba environment and Singularity image used
to run SKNanoAnalyzer on the SNU cluster.

### What is micromamba?

Micromamba is a lightweight conda-compatible package manager. It installs the
compiler, ROOT, correction libraries, and Python tools as one versioned
environment.

### What is singularity?

Singularity, also distributed as Apptainer, gives batch nodes a consistent base
operating system. The image mounts the shared `Nano` environment, keeping the
large package environment outside the image and available to every worker.

## Setting up micromamba
### Install micromamba
micromamba can be installed easily by following the instructions on the [official micromamba documentation](https://mamba.readthedocs.io/en/latest/installation/micromamba-installation.html). Simply follow the instructions exactly as provided, and no particular problems should arise.

```bash
"${SHELL}" <(curl -L micro.mamba.pm/install.sh)
```
This command executes install.sh in the current shell environment. When you execute this command, you will be prompted with the following questions.

```
Micromamba binary folder? [~/.local/bin]
```
This is a question about where to place the micromamba package manager. ***Do not use the default path.*** Instead, I recommend using `/data6/Users/your_id/micromamba_bin` as the installation path. This is because the default path is under home directory, which cluster nodes do not have access to. 

```
Init shell (zsh)? [Y/n]
```
Choose `Y` to initialize micromamba for your shell (zsh in this case). This will add necessary configurations to your shell profile file (e.g., `.zshrc`).

```
Configure conda-forge? [Y/n] 
```
Choose `Y` to add the conda-forge channel to your micromamba configuration. This channel contains a wide variety of packages that you may need.


```
Prefix location? [~/micromamba]
```
***Do not use the default path.*** as same reason mentioned above. Instead, I recommend using `/data6/Users/your_id/micromamba_envs` as the installation path for micromamba environments.

After completing the installation, this script automatically adds the micromamba initialization command to your shell profile file (e.g., `.zshrc`, `.bashrc`). like below:

```bash
# >>> mamba initialize >>>
# !! Contents within this block are managed by 'micromamba shell init' !!
export MAMBA_EXE='/data6/Users/your_id/micromamba_bin/micromamba';
export MAMBA_ROOT_PREFIX='/data6/Users/your_id/micromamba_envs';
__mamba_setup="$("$MAMBA_EXE" shell hook --shell bash --root-prefix "$MAMBA_ROOT_PREFIX" 2> /dev/null)"
if [ $? -eq 0 ]; then
    eval "$__mamba_setup"
else
    alias micromamba="$MAMBA_EXE"  # Fallback on help from micromamba activate
fi
unset __mamba_setup
# <<< mamba initialize <<<
```
After restarting your terminal, you can verify the installation by running:
```bash
micromamba --version
```
This should display the installed version of micromamba.

### Install required packages

From the repository root, create the shared `Nano` environment from the pinned
Linux lock file:

```bash
micromamba create -n Nano -f docs/Nano-linux-64.lock
micromamba activate Nano
```

If a locked package URL is no longer available, use the solver input with
`micromamba env create -f docs/Nano.yml`. Do not upgrade ROOT, Abseil,
correctionlib, or the compiler independently; they form one C++ ABI-compatible
toolchain.

## Setting up Singularity
### Make Singularity image

Now we need to create the Singularity image.
Fundamentally, this is conceptually almost identical to building a new computer and installing the OS fresh. Therefore, you must configure the necessary environment settings when creating the image. Then, each time you run the image, it's like booting up a new computer with exactly this configuration already complete.

This is done through the `.def` file. Save the following content as `SKNANOAnalyzer.def`.

```def
Bootstrap: docker
From: almalinux:9

%labels
    MAINTAINER "Yeonjoon Kim <yeonjoon.kim@cern.ch>"
    DESCRIPTION "Snapshot of local micromamba env via conda-pack (EL9 base)."

%post
    set -eux
    dnf -y install zsh bzip2
    mkdir -p /opt/conda && cd /opt/conda
    curl -Ls https://micro.mamba.pm/api/micromamba/linux-64/latest | tar -xvj bin/micromamba
    echo 'export PATH=/opt/conda/bin:$PATH' > /etc/profile.d/conda.sh
    echo 'export MAMBA_ROOT_PREFIX=/opt/conda' >> /etc/profile.d/conda.sh
    mkdir -p /opt/conda/envs
    ln -sfn [your_path_to_envs]/envs/Nano /opt/conda/envs/Nano || true
    chown -R root:users /opt/conda && chmod -R 775 /opt/conda

%environment
    export CONDA_PREFIX=/opt/conda/envs/Nano
    export PATH=$CONDA_PREFIX/bin:/opt/conda/bin:$PATH
    export LD_LIBRARY_PATH=$CONDA_PREFIX/lib:$CONDA_PREFIX/lib64:${LD_LIBRARY_PATH}
    export PYTHONNOUSERSITE=1

%runscript
    exec /bin/zsh
```

This file downloads the image corresponding to Almalinux 9, then executes the script in the `%post` section to build the image.
```bash
curl -Ls https://micro.mamba.pm/api/micromamba/linux-64/latest | tar -xvj bin/micromamba
```
Above command installs micromamba inside the Singularity image. However, in this micromamba environment, the libraries we installed earlier—such as `root` and `correctionlib`—are not present.
We don't want to rebuild the image every time we install or update a new package, so let's ensure the micromamba installed inside the image can access the `Nano` environment we created earlier.
```bash
ln -sfn [your_path_to_envs]/envs/Nano /opt/conda/envs/Nano || true
echo 'export PATH=/opt/conda/bin:$PATH' > /etc/profile.d/conda.sh
echo 'export MAMBA_ROOT_PREFIX=/opt/conda' >> /etc/profile.d/conda.sh
```
The above commands create a symbolic link to the `Nano` environment we created earlier, allowing us to use all the packages installed in that environment within the Singularity image.

After saving the above content as `SKNANOAnalyzer.def`, you can build the Singularity image by running the following command in the terminal:
```bash
apptainer build SKNANOAnalyzer.sif SKNANOAnalyzer.def
```
After building the image, return to [Getting Started](GettingStarted.md) to
configure, build, and run SKNanoAnalyzer.
