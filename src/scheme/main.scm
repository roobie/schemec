
(define-module (omni main)
  #:use-module (ice-9 format)
  #:export (omni/init))

(define (omni/init)
  (format #t "~a~%" "Initializing OMNI")
  (ecs/create-entity)
  (format #t "~a~%" "We are ready.")
  )
