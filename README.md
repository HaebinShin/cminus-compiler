# Intro
기초컴파일러(Fundamentals of Compiler Construction) 수업의 컴파일러 구현 실습 프로젝트

# Phases
C- 라는 C의 부분집합 언어의 컴파일러를 구현한다.
# Project 1: Lexical Analyzer
* 교재에 수록된 tiny 컴파일러의 코드를 참고하여 작성
* tokenization은 `flex` 라는 scanner generator에서 나온 scanner를 활용한다.

# Utilities
## Docker
`./test.sh`로 테스트함
`INPUT_SOURCE` 환경변수에 테스트에 쓰일 `sample_inputs` 아래의 소스코드명을 지정함