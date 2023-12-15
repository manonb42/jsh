echo "---------- TEST [<] ----------"
./redirectionner ls '<' alo
./redirectionner pwd '<' alo
./redirectionner cat '<' alo
echo "------------------------------"

echo "---------- TEST [>] ----------"
./redirectionner ls '>' alocp
echo "result :" && cat alocp && rm alocp
#################################
./redirectionner pwd '>' alocp
echo "result :" && cat alocp
#################################
./redirectionner cat '>' alocp
echo "result :" && cat alocp && rm alocp
echo "------------------------------"

echo "---------- TEST [>|] ----------"
./redirectionner ls '>|' alocp
echo "result :" && cat alocp
#################################
./redirectionner pwd '>|' alocp
echo "result :" && cat alocp && rm alocp
#################################
# ./redirectionner cat '>|' alocp && rm alocp
echo "------------------------------"

echo "---------- TEST [>>] ----------"
./redirectionner pwd '>>' alocp
echo "result :" && cat alocp
#################################
./redirectionner ls '>>' alocp
echo "result :" && cat alocp
#################################
# ./redirectionner cat '>>' alocp
echo "------------------------------"

