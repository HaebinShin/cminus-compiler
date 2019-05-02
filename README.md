# Intro
기초컴파일러(Fundamentals of Compiler Construction) 수업의 컴파일러 구현 실습 프로젝트

# Phases
C- 라는 C의 부분집합 언어의 컴파일러를 구현한다.
# Project 1: Lexical Analyzer
* 교재에 수록된 tiny 컴파일러의 코드를 참고하여 작성
* tokenization은 `flex` 라는 scanner generator에서 나온 scanner를 활용한다.
# Proejct 2: LALR Parser

# Utilities
## Docker
* build image: `docker build --rm -t cspro_mimic .`
* run image: `docker run -v $PWD:/project -it cspro_mimic bash`
* start the exited one: `docker start <name>` and `docker attach <name>`

# References
http://aquamentus.com/flex_bison.html
https://stackoverflow.com/questions/38009696/token-redefinition-flex-bison