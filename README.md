# RegexEngine
Simple regular expression engine implemented using Thompson NFA (Nondeterministic Finite Automata).

## Supported Features
- Literal matches ('a', '\n' etc.)
- Escaped literal matches ('\(', '\\' etc.)
- Any-literal matches (.)
- Greedy quantifiers (+, ?, \*)
- Character class ([a-z], [a-zA-Z] etc.)
- Counted repititions ({n}, {n,}, {n,m})
- Greedy submatch extraction

## Unsupported Features
- Regex validility check
- Ungreedy quantifiers
- Unanchored matches
- Backreferences
