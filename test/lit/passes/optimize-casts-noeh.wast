;; NOTE: Assertions have been generated by update_lit_checks.py and should not be edited.
;; RUN: wasm-opt %s --optimize-casts --enable-reference-types --enable-gc --enable-tail-call -S -o - | filecheck %s

(module
  ;; CHECK:      (type $A (struct ))
  (type $A (struct))

  ;; CHECK:      (func $yes-past-call (type $ref|struct|_=>_none) (param $x (ref struct))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast (ref $A)
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (call $none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $yes-past-call (param $x (ref struct))
    (drop
      (ref.cast (ref $A)
        (local.get $x)
      )
    )
    ;; The call in the middle does not stop us from helping the last get, since
    ;; EH is not enabled. The last get will flip from $x to a new tee of the
    ;; cast.
    (call $none)
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $yes-past-return_call (type $ref|struct|_=>_none) (param $x (ref struct))
  ;; CHECK-NEXT:  (local $1 (ref $A))
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.tee $1
  ;; CHECK-NEXT:    (ref.cast (ref $A)
  ;; CHECK-NEXT:     (local.get $x)
  ;; CHECK-NEXT:    )
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT:  (return_call $none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (local.get $1)
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $yes-past-return_call (param $x (ref struct))
    (drop
      (ref.cast (ref $A)
        (local.get $x)
      )
    )
    ;; The call_return in the middle does not stop us from optimizing, since
    ;; after we branch out it doesn't matter what we have below.
    (return_call $none)
    (drop
      (local.get $x)
    )
  )

  ;; CHECK:      (func $none (type $none_=>_none)
  ;; CHECK-NEXT:  (nop)
  ;; CHECK-NEXT: )
  (func $none
    ;; Helper for the above.
  )
)
