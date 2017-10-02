#!/usr/bin/env zsh
count=0
rm -rf jobs
mkdir jobs
rm -rf batchlog
mkdir batchlog

DIR=`pwd`

for sigSample in HH_sm_eft_1M.sample; do
    for i in {0..9}; do
        echo '#!/bin/sh' > jobs/job$count.sh
        echo '#PBS -l cput=02:00:00' >> jobs/job$count.sh
        echo '#PBS -l walltime=02:00:00' >> jobs/job$count.sh
        echo "#PBS -o $DIR/batchlog/" >> jobs/job$count.sh
        echo "#PBS -e $DIR/batchlog/" >> jobs/job$count.sh
        echo 'cd $PBS_O_WORKDIR' >> jobs/job$count.sh
        echo '. ../setupEnv.sh' >> jobs/job$count.sh
        echo "./HH4b cards/baseline/baseline_noPU.run cards/baseline/$sigSample $i" >> jobs/job$count.sh
        chmod +x jobs/job$count.sh
        qsub jobs/job$count.sh
	echo "Job $count submitted"

        count=$[$count+1]
    done
done
for bkgSample in cards/baseline/SHERPA*.sample; do
    for i in {0..29}; do
       echo '#!/bin/sh' > jobs/job$count.sh
       echo '#PBS -l cput=02:00:00 -l walltime=02:00:00' >> jobs/job$count.sh
       echo '#PBS -l cput=02:00:00' >> jobs/job$count.sh
       echo '#PBS -l walltime=02:00:00' >> jobs/job$count.sh
       echo "#PBS -o $DIR/batchlog/" >> jobs/job$count.sh
       echo "#PBS -e $DIR/batchlog/" >> jobs/job$count.sh
       echo 'cd $PBS_O_WORKDIR' >> jobs/job$count.sh
       echo '. ../setupEnv.sh' >> jobs/job$count.sh
       echo "./HH4b cards/baseline/baseline_noPU.run $bkgSample $i" >> jobs/job$count.sh
       chmod +x jobs/job$count.sh
       qsub jobs/job$count.sh
       count=$[$count+1]
       echo "Job $count submitted"
    done
done
