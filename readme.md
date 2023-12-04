
# Analyzer for custom compiler

A program in C++ that implements the analyzer step of custom compiler of the TINY programming language based on the grammar rules of the language.



## Program
- Program input: 
    The scanner will take the path of a source code file written in    the TINY programming language as an input; You can find an example of an input file in the file called `input.txt`.
- Program output: 
    The program should output the terminal(leaf) nodes of the parse tree of the input file to the terminal or throw an exception once an error was found. 
## TINY language grammar rules:

- program -> stmtseq
- stmtseq -> stmt { ; stmt }
- stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
- ifstmt -> if expr then stmtseq [ else stmtseq ] end
- repeatstmt -> repeat stmtseq until expr
- assignstmt -> identifier := expr
- readstmt -> read identifier
- writestmt -> write expr
- expr -> mathexpr [ (<|=) mathexpr ]
- mathexpr -> term { (+|-) term }    left associative
- term -> factor { (*|/) factor }    left associative
- factor -> newexpr { ^ newexpr }    right associative
- newexpr -> ( mathexpr ) | number | identifier

## 🚀 About Me
Aspiring and adept Software Engineer, leveraging a progression from C++ to advanced proficiency in Java and Kotlin for Android development. 

## Feedback

If you have any feedback, please reach me out at 
[![linkedin](https://img.shields.io/badge/linkedin-0A66C2?style=for-the-badge&logo=linkedin&logoColor=white)](https://www.linkedin.com/in/abdelaziz-ashraf-4825ba173/)

