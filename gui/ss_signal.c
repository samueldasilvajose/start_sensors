#include "ss_signal.h"
#include "ss_signal_types.h"


// Array de IDs dos sinais
static guint signals[SS_SIGNAL_UPDATE_BACKEND_COUNT] = {0};

// Estrutura da instância
struct _SsController
{
    GObject parent_instance;
};

// Define o tipo Controller
static void ss_controller_init(SsController *self);
static void ss_controller_class_init(SsControllerClass *klass);

// Define o tipo Controller
G_DEFINE_TYPE(SsController, ss_controller, G_TYPE_OBJECT)

// Inicialização da instância
static void ss_controller_init(SsController *self) { (void) self; }

// Inicialização da classe: registrar sinais
static void ss_controller_class_init(SsControllerClass *klass)
{
    for (int i = 0; i < SS_SIGNAL_UPDATE_BACKEND_COUNT; i++)
    {
        signals[i] = g_signal_new(
            ss_singnal_to_string(i),       // nome do sinal
            G_TYPE_FROM_CLASS(klass),      // tipo da classe
            G_SIGNAL_RUN_FIRST,            // quando o sinal é emitido
            0,                             // offset do handler
            NULL, NULL,                    // funções de accumulator
            g_cclosure_marshal_VOID__POINTER, // marshal correto para gpointer
            G_TYPE_NONE,                   // tipo de retorno do sinal
            1,                             // número de argumentos
            G_TYPE_POINTER                  // tipo do argumento
        );
    }
}

SsController *
ss_controller_new()
{
    return g_object_new(TYPE_SS_CONTROLLER, NULL);
}

inline void
_ss_controller_close_app(SsController *self, gpointer data)
{
    (void) data;
    
    g_signal_emit(self, signals[SS_SIGNAL_UPDATE_BACKEND_CLOSE_APPLICATION], 0, NULL);
}

// Funções que o backend chama para emitir sinais
inline void
_ss_controller_backend_stack(SsController *self, gpointer data)
{
    g_signal_emit(self, signals[SS_SIGNAL_UPDATE_BACKEND_STACK], 0, data);
}


inline void
_ss_controller_backend_notify(SsController *self, gpointer data)
{
    g_signal_emit(self, signals[SS_SIGNAL_UPDATE_BACKEND_NOTIFICATION], 0, data);
}
