echo -n "Input R to start reader program, input W to start writer program: "
read PROG

touch file.txt

if [[ $PROG =~ ^[R]+$ ]]
then
    echo 'Starting Reader program'
    PROG='reader'
elif [[ $PROG =~ ^[W]+$ ]]
then
    echo 'Starting Writer program'
    PROG='writer'
else
    echo 'You should enter R or W!'
    exit 1
fi


echo "Compiling program..."
g++ -c ${PROG}.cpp

echo 'Linking...'
g++ -o ${PROG} ${PROG}.o -lpthread

echo "Starting program execution..."
echo '=================================================='
echo ''

./${PROG}

echo ''
echo '=================================================='
echo "Program finished!"

echo ''
echo 'Cleaning directory, deleting .o and executable...'
rm ./${PROG}.o
rm ./${PROG}
rm ./file.txt

echo 'Done!'
