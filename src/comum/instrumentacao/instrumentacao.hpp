#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

class Instrumentacao {
public:
    using valor_t = std::int64_t;

    
    Instrumentacao();


    Instrumentacao criar_subinstrumentacao(std::string_view nome) const;

    void incrementar(std::string_view nome, valor_t delta = 1);

    void definir(std::string_view nome, valor_t valor);

    std::optional<valor_t> obter(std::string_view nome) const;

    valor_t obter_ou_zero(std::string_view nome) const;

    bool existe(std::string_view nome) const;

    std::map<std::string, valor_t> exportar() const;

    void limpar();

private:
    struct EstadoCompartilhado {
        mutable std::mutex mutex;
        std::map<std::string, valor_t> metricas;
    };

    explicit Instrumentacao(std::shared_ptr<EstadoCompartilhado> estado, std::string prefixo);

    std::shared_ptr<EstadoCompartilhado> estado_;
    std::string prefixo_;

    std::string nome_completo(std::string_view nome) const;
    static void validar_nome(std::string_view nome);
};
