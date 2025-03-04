;; NOTE: Assertions have been generated by update_lit_checks.py --all-items and should not be edited.

;; RUN: wasm-opt -all %s -S -o - | filecheck %s

;; Check that the deprecated `array.init_static` alias for `array.new_fixed` is
;; parsed correctly.

(module
  ;; CHECK:      (type $none_=>_none (func))

  ;; CHECK:      (type $array (array i32))
  (type $array (array i32))
  ;; CHECK:      (func $test (type $none_=>_none)
  ;; CHECK-NEXT:  (drop
  ;; CHECK-NEXT:   (array.new_fixed $array 2
  ;; CHECK-NEXT:    (i32.const 0)
  ;; CHECK-NEXT:    (i32.const 1)
  ;; CHECK-NEXT:   )
  ;; CHECK-NEXT:  )
  ;; CHECK-NEXT: )
  (func $test
    (drop
      (array.init_static $array
        (i32.const 0)
        (i32.const 1)
      )
    )
  )
)
