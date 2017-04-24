Function IsPrime(Number As Integer) As Integer
    Dim Divisor As Integer
    Let Divisor = 2
    Do While Divisor < Number
        If Number / Divisor * Divisor = Number Then
            Return 0
        End If
        Let Divisor = Divisor + 1
    Loop
    Return 1
End Function

Function Main() As Integer
    Dim N As Integer
    Dim Max As Integer
    Input max
    Let N = 2
    Do While N <= Max
        If IsPrime(N) = 1 Then
            Print N
        End If
        Let N = N + 1
    Loop
    Return 0
End Function
