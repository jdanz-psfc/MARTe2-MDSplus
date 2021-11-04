# set environment for RABBIT on SPC lac cluster

# functions
set_lac8_env()
{
matlab850cmd="/usr/local/MATLAB/R2017a/bin/matlab -c /etc/network.lic -nodesktop -softwareopengl -nosplash"
export IMPI_PATH=/opt/intel/impi/5.1.1.109/lib64
}

set_scd_env()
{
matlab850cmd="/usr/local/MATLAB/R2015a/bin/matlab -c /etc/network.lic -nodesktop -softwareopengl -nosplash"
export IMPI_PATH=/opt/intel2018/impi/2018.0.128/lib64
}

set_toki_env()
{
matlab850cmd="matlab -nosplash -nodesktop"
export IMPI_PATH=/afs/ipp/common/soft/intel/ics2013/impi/5.0.0.028/lib64/
. /etc/profile.d/modules.sh # activate modules on toki
echo "Loading modules.."
module purge
module load intel/14.0
module load matlab/2015a
module load impi
module load hdf5-mpi
module load netcdf-mpi
}

set_rats_env()
{
matlab850cmd="ML2019B-UNIPD -d"
export IMPI_PATH=/usr/local/intel/impi/5.0.2.044/intel64/lib64
}

setup_RABBIT()
{
# add lib path to LD_LIBRARY_PATH
export LD_LIBRARY_PATH=${RABBIT_PATH}/lib:${IMPI_PATH}:${LD_LIBRARY_PATH}

# some displaying
echo "Setting RABBIT_PATH ${RABBIT_PATH}"
echo "Setting IMPI_PATH ${IMPI_PATH}"
echo "Setting LD_LIBRARY_PATH ${LD_LIBRARY_PATH}"
}

# MAIN SCRIPT
echo RABBIT environment setup script
echo "HOSTNAME is $HOSTNAME"
SCRIPTPATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd)"
export RABBIT_PATH=$SCRIPTPATH

if [ "$HOSTNAME" == "lac8.epfl.ch" ]; then
set_lac8_env
elif [ "$HOSTNAME" == "spcpc478.epfl.ch" ]; then
set_scd_env
elif [[ "$HOSTNAME" == "toki"* ]]; then
set_toki_env
elif [[ "$HOSTNAME" == "rat"* ]]; then
set_rats_env
else
echo "RABBIT set up not defined for this hostname: $HOSTNAME"
exit 0
fi

setup_RABBIT


