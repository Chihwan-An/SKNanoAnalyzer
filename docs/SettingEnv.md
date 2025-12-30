# Setting up micromamba + singularity environment
 
## Overview
This section describes how to set up the micromamba + singularity environment for running SKNANOAnalyzer.
### What is micromamba?
One who deals with HEP analysis needs several different software packages and libraries. Install all required packages manually with correct dependencies is a tedious task. Hence one needs a package manager that can handle the installation and dependency resolution automatically. Conda is one of the most popular package managers in various scientific fields including HEP. Micromamba is a lightweight version of conda that provides almost the same functionalities as conda but with a smaller size and faster performance. 

### What is singularity?
Singularity is like a box that contains everything your program needs to run—the program itself, libraries, and settings. Because everything is packed together, you can run it on different computers and it will work the same way everywhere. That’s why it’s widely used on servers and supercomputers to avoid the “it works on my computer but not on this one” problem. 

Typically, cluster environments adopt a method of distributing the same task across multiple computers for parallel processing. To manage the different environments of multiple computers as one and eliminate potential errors, this analyzer also adopts a method of running cluster jobs within a Singularity image.

To add an unnecessary footnote, running jobs within Singularity isn't strictly necessary since all clusters on the SNU server are well-configured to share the same environment. However, the situation may differ in other larger cluster environments. Furthermore, since setting up the environment cleanly within an image is a better choice for preventing unnecessary conflicts, this project opts to run tasks from within the image.

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
Now that micromamba is installed, you can create a new environment and install the required packages for SKNANOAnalyzer. Create a new environment named `Nano` as follows:
```bash
micromamba create -n Nano python=3.12
```
Then you can activate the environment:
```bash
micromamba activate Nano
```
Then for now, micromamba automatically helps run all required packages in its environment. Install the required packages by running:
```bash
micromamba install -n Nano \
  root=6.34.4 xrootd=5.8.1 pythia8=8.312 fastjet-cxx=3.4.3 siscone=3.0.6 correctionlib=2.6.4 awkward=2.7.2 awkward-cpp=43 uproot=5.5.1 vector-classes=1.4.5 vdt \
  htcondor=24.8.1 htcondor-classads=24.8.1 htcondor-cli=24.8.1 htcondor-utils=24.8.1 python-htcondor=24.8.1 scitokens-cpp \
  numpy=2.0.2 numba=0.60.0 sympy=1.13.3 ml_dtypes \
  onnxruntime=1.19.2 onnxruntime-cpp=1.19.2 \
  jupyter jupyterlab notebook ipykernel ipywidgets ipyparallel \
  matplotlib-base=3.10.0 mplhep=0.3.55 \
  cmake ninja gcc_linux-64 gxx_linux-64 gfortran_linux-64 binutils_linux-64 \
  gitpython rich tqdm
```

The packages listed above are not strictly required, but they include useful tools that can be beneficial. If installation fails, you may disable version specification. However, we strongly recommend matching the version for `root` specifically.

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
Now you finish to make a basic settings for run SKNANOAnalyzer. Back to [Getting Started](GettingStarted.md), and continue the instructions from "Setting up the environment" section.